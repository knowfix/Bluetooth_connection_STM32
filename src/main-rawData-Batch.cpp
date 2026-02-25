/*
 * main_aggregated.cpp - EEG 100Hz with Packet Aggregation
 * STM32F401 + JDY-23
 * Author: Ahmad Taufiq
 * 
 * Strategy: Buffer 3 samples, send as one batch packet
 * Benefits: 67% less BLE transmissions, < 1% packet loss
 */

#include <Arduino.h>
#include "Bluetooth.h"
#include "eeg_dummy.h"

// ===== PIN CONFIG =====
#define PIN_BT_RX      PB7
#define PIN_BT_TX      PB6
#define PIN_BT_PWR     PA8
#define PIN_BT_STAT    PB10
#define PIN_BT_RST     PC9
#define PIN_BT_LED     PC12

// ===== EEG CONFIG =====
#define EEG_CHANNELS       16
#define EEG_INTERVAL_MS    10      // Generate at 100 Hz
#define BATCH_SIZE         3       // Send 3 samples per batch
#define BATCH_TIMEOUT_MS   35      // Send partial batch if timeout

// ===== PACKET MARKERS =====
#define SINGLE_HEADER  0xAA        // Single packet header (for SYNC)
#define BATCH_HEADER   0xBB        // Batch packet header
#define EEG_FOOTER     0x55
#define EEG_SYNC       0xF0

// ===== PACKET SIZES =====
// Single sample: [Seq(1)][CH0-15 (32 bytes)][CRC(1)] = 34 bytes
// Batch packet: [0xBB][Count][Sample1][Sample2][Sample3][CRC][0x55]
#define SINGLE_SAMPLE_SIZE 34
#define MAX_BATCH_SIZE     5
#define BATCH_PACKET_SIZE  (2 + (MAX_BATCH_SIZE * SINGLE_SAMPLE_SIZE) + 2)

// ===== DEBUG CONFIG =====
#define DEBUG_ENABLED      1
#define DEBUG_INTERVAL     50      // Log every 50 packets
#define DEBUG_TIMING       1

// ===== MODE =====
enum SystemMode {
    MODE_TEXT,
    MODE_EEG
};

SystemMode currentMode = MODE_TEXT;

// ===== BLUETOOTH =====
BluetoothJDY23 bt(
    PIN_BT_RX, PIN_BT_TX,
    PIN_BT_PWR, PIN_BT_STAT,
    PIN_BT_RST, PIN_BT_LED
);

// ===== EEG STATE =====
uint8_t  eegSeq = 0;
uint32_t totalSamplesSent = 0;
uint32_t totalBatchesSent = 0;
unsigned long lastEEGGenerate = 0;
bool justConnected = true;

// ===== BATCH BUFFER =====
uint8_t batchPacket[BATCH_PACKET_SIZE];
uint8_t sampleQueue[MAX_BATCH_SIZE][SINGLE_SAMPLE_SIZE];
uint8_t queueCount = 0;
unsigned long lastBatchSend = 0;

// ===== TIMING STATS =====
uint32_t lastSampleCount = 0;
unsigned long totalProcessingTime = 0;
uint32_t processingCount = 0;

// ===== CRC8 =====
uint8_t crc8(uint8_t *data, uint8_t len)
{
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++)
        crc ^= data[i];
    return crc;
}

// ===== DEBUG =====
void debugPrintBatch(uint8_t count, unsigned long processingTime)
{
#if DEBUG_ENABLED
    static uint32_t debugCounter = 0;
    debugCounter += count;
    
    if (debugCounter % DEBUG_INTERVAL < count) {
        char buf[150];
        snprintf(buf, sizeof(buf), 
            "[BATCH] #%lu | Samples:%d | TotalSamples:%lu | Time:%lu ms | Proc:%.2f ms\n",
            totalBatchesSent, count, totalSamplesSent, millis(), processingTime / 1000.0
        );
        Serial.print(buf);
    }
#endif
}

// ===== SEND SYNC PACKET =====
void sendSyncPacket()
{
    uint8_t syncPacket[36];
    memset(syncPacket, 0, 36);
    
    syncPacket[0]  = SINGLE_HEADER;
    syncPacket[1]  = EEG_SYNC;
    syncPacket[34] = crc8(syncPacket, 34);
    syncPacket[35] = EEG_FOOTER;

    bt.send(syncPacket, 36);
    Serial.println("[BT] SYNC packet sent");
}

// ===== RESET EEG STATE =====
void resetEEGState()
{
    eegSeq = 0;
    totalSamplesSent = 0;
    totalBatchesSent = 0;
    queueCount = 0;
    lastEEGGenerate = millis();
    lastBatchSend = millis();
    eegDummyReset();
    justConnected = true;
    
    lastSampleCount = 0;
    totalProcessingTime = 0;
    processingCount = 0;

    Serial.println("[EEG] State reset");
}

// ===== GENERATE AND QUEUE SAMPLE =====
void generateAndQueueSample()
{
    if (queueCount >= BATCH_SIZE) {
        Serial.println("[WARN] Queue full, skipping sample");
        return;
    }
    
    unsigned long startTime = micros();
    
    float   eegFloat[EEG_CHANNELS];
    int16_t eegInt[EEG_CHANNELS];

    // Generate EEG data
    eegDummyGenerate16(eegFloat);

    // Build sample in queue
    sampleQueue[queueCount][0] = eegSeq;  // Sequence number

    // Pack 16 channels (32 bytes)
    for (int i = 0; i < EEG_CHANNELS; i++) {
        int32_t v = (int32_t)roundf(eegFloat[i]);
        if (v > 32767) v = 32767;
        if (v < -32768) v = -32768;
        eegInt[i] = (int16_t)v;

        int idx = 1 + (i * 2);
        sampleQueue[queueCount][idx]     = eegInt[i] & 0xFF;
        sampleQueue[queueCount][idx + 1] = (eegInt[i] >> 8) & 0xFF;
    }

    // Individual sample CRC (33 bytes: seq + channels)
    sampleQueue[queueCount][33] = crc8(sampleQueue[queueCount], 33);

    unsigned long processingTime = micros() - startTime;
    totalProcessingTime += processingTime;
    processingCount++;

    eegSeq++;
    queueCount++;
}

// ===== SEND BATCHED PACKET =====
void sendBatchedPacket()
{
    if (queueCount == 0) return;
    if (!bt.isConnected()) return;
    
    unsigned long startTime = micros();
    
    // Build batch packet header
    batchPacket[0] = BATCH_HEADER;  // 0xBB - indicates batched packet
    batchPacket[1] = queueCount;    // Number of samples in this batch
    
    // Copy all queued samples
    for (int i = 0; i < queueCount; i++) {
        memcpy(&batchPacket[2 + (i * SINGLE_SAMPLE_SIZE)], 
               sampleQueue[i], 
               SINGLE_SAMPLE_SIZE);
    }
    
    // Overall batch CRC (everything except CRC and footer)
    int dataSize = 2 + (queueCount * SINGLE_SAMPLE_SIZE);
    batchPacket[dataSize] = crc8(batchPacket, dataSize);
    batchPacket[dataSize + 1] = EEG_FOOTER;
    
    // Send to Bluetooth
    int packetSize = dataSize + 2;
    bt.send(batchPacket, packetSize);
    
    unsigned long processingTime = micros() - startTime;
    
    // Update stats
    totalSamplesSent += queueCount;
    totalBatchesSent++;
    
    // Debug output
    debugPrintBatch(queueCount, processingTime);
    
    // Reset queue
    queueCount = 0;
    lastBatchSend = millis();
}

// ===== TIMING STATISTICS =====
void printTimingStats()
{
#if DEBUG_TIMING
    static unsigned long lastPrint = 0;
    unsigned long now = millis();
    
    // Print every 5 seconds
    if (now - lastPrint >= 5000) {
        uint32_t samplesInPeriod = totalSamplesSent - lastSampleCount;
        float actualHz = samplesInPeriod / 5.0;
        
        float avgProcessingMs = 0;
        if (processingCount > 0) {
            avgProcessingMs = (totalProcessingTime / (float)processingCount) / 1000.0;
        }
        
        float avgBatchSize = totalBatchesSent > 0 ? 
            (float)totalSamplesSent / totalBatchesSent : 0;
        
        float bleTransmissionsPerSec = totalBatchesSent / ((now - lastPrint + 5000) / 1000.0);
        
        Serial.println("\n========== TIMING STATS ==========");
        Serial.print("Target: 100 Hz | Actual: ");
        Serial.print(actualHz, 2);
        Serial.print(" Hz");
        
        if (actualHz < 95) {
            Serial.println(" ⚠️ TOO SLOW");
        } else if (actualHz > 105) {
            Serial.println(" ⚠️ TOO FAST");
        } else {
            Serial.println(" ✅ OK");
        }
        
        Serial.print("Avg Sample Processing: ");
        Serial.print(avgProcessingMs, 2);
        Serial.println(" ms");
        
        Serial.print("Total Samples Sent: ");
        Serial.println(totalSamplesSent);
        
        Serial.print("Total Batches Sent: ");
        Serial.println(totalBatchesSent);
        
        Serial.print("Avg Batch Size: ");
        Serial.print(avgBatchSize, 2);
        Serial.println(" samples");
        
        Serial.print("BLE Transmissions/sec: ");
        Serial.print(bleTransmissionsPerSec, 1);
        Serial.print(" (vs 100 without batching)");
        Serial.println();
        
        Serial.print("BLE Load Reduction: ");
        Serial.print((1.0 - (bleTransmissionsPerSec / 100.0)) * 100, 1);
        Serial.println("%");
        
        float dataRateKBps = (actualHz * 36) / 1024.0;
        Serial.print("Data Rate: ");
        Serial.print(dataRateKBps, 2);
        Serial.println(" KB/s");
        
        Serial.println("==================================\n");
        
        lastPrint = now;
        lastSampleCount = totalSamplesSent;
        totalProcessingTime = 0;
        processingCount = 0;
    }
#endif
}

// ===== HANDLE COMMANDS =====
void onBluetoothData(String data)
{
    data.trim();
    data.toUpperCase();

    Serial.print("[CMD] ");
    Serial.println(data);

    if (data == "EEG_ON" || data == "START") {
        currentMode = MODE_EEG;
        resetEEGState();
        bt.println("OK:EEG_ON");
        Serial.println("[MODE] Batched EEG streaming @ 100 Hz STARTED");
        Serial.print("[MODE] Batch size: ");
        Serial.print(BATCH_SIZE);
        Serial.println(" samples");
    }
    else if (data == "EEG_OFF" || data == "STOP") {
        // Send any remaining samples in queue
        if (queueCount > 0) {
            Serial.print("[INFO] Sending remaining ");
            Serial.print(queueCount);
            Serial.println(" samples");
            sendBatchedPacket();
        }
        
        currentMode = MODE_TEXT;
        bt.println("OK:EEG_OFF");
        Serial.println("[MODE] Streaming STOPPED");
        
        // Print final stats
        Serial.println("\n========== FINAL SESSION STATS ==========");
        Serial.print("Total samples sent: ");
        Serial.println(totalSamplesSent);
        Serial.print("Total batches sent: ");
        Serial.println(totalBatchesSent);
        if (totalBatchesSent > 0) {
            Serial.print("Average batch size: ");
            Serial.println((float)totalSamplesSent / totalBatchesSent, 2);
        }
        Serial.println("=========================================\n");
    }
    else if (data == "RESET") {
        currentMode = MODE_TEXT;
        resetEEGState();
        bt.println("OK:RESET");
        Serial.println("[MODE] System RESET");
    }
    else if (data == "STATUS") {
        bt.println("MODE:" + String(currentMode == MODE_EEG ? "EEG_BATCH" : "TEXT"));
        
        Serial.print("[STATUS] Mode: ");
        Serial.println(currentMode == MODE_EEG ? "EEG_BATCH" : "TEXT");
        Serial.print("[STATUS] Samples sent: ");
        Serial.println(totalSamplesSent);
        Serial.print("[STATUS] Batches sent: ");
        Serial.println(totalBatchesSent);
        Serial.print("[STATUS] Queue: ");
        Serial.print(queueCount);
        Serial.print("/");
        Serial.println(BATCH_SIZE);
        Serial.print("[STATUS] Sequence: ");
        Serial.println(eegSeq);
    }
    else {
        bt.println("ERR:UNKNOWN_CMD");
        Serial.println("[CMD] Unknown command");
    }
}

// ===== SETUP =====
void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n========================================");
    Serial.println("  STM32 EEG @ 100Hz - AGGREGATED MODE");
    Serial.println("========================================");
    Serial.println("Configuration:");
    Serial.println("  - Sample Rate: 100 Hz");
    Serial.println("  - Interval: 10ms per sample");
    Serial.println("  - Baudrate: 38400");
    Serial.print("  - Batch Size: ");
    Serial.print(BATCH_SIZE);
    Serial.println(" samples");
    Serial.print("  - Batch Frequency: ~");
    Serial.print(100.0 / BATCH_SIZE, 1);
    Serial.println(" Hz");
    Serial.print("  - BLE Load Reduction: ~");
    Serial.print((1.0 - (1.0 / BATCH_SIZE)) * 100, 0);
    Serial.println("%");
    Serial.print("  - Batch Packet Size: ");
    Serial.print(2 + (BATCH_SIZE * 34) + 2);
    Serial.println(" bytes");
    Serial.println("");
    Serial.println("Benefits:");
    Serial.println("  ✅ 67% less BLE transmissions");
    Serial.println("  ✅ < 1% expected packet loss");
    Serial.println("  ✅ Better stability");
    Serial.println("  ✅ More efficient");
    Serial.println("========================================\n");

    // Use optimal baudrate from previous tests
    bt.begin(38400);
    bt.onDataReceived(onBluetoothData);
    bt.setAutoEcho(false);
    
    Serial.println("[BT] Initialized @ 38400 baud");
    Serial.println("[INFO] Make sure JDY-23 has AT+MTU=2");
    Serial.println("[INFO] Android app must support batch protocol");
    Serial.println("");
}

// ===== MAIN LOOP =====
void loop()
{
    bt.update();

    // Connection monitoring
    static bool lastConnected = false;
    bool nowConnected = bt.isConnected();

    if (nowConnected && !lastConnected) {
        Serial.println("\n[BT] *** CONNECTED ***");
        Serial.print("[BT] Connection time: ");
        Serial.print(millis());
        Serial.println(" ms");
        
        resetEEGState();
        sendSyncPacket();
        
        Serial.println("[BT] Ready for batched streaming\n");
    }
    
    if (!nowConnected && lastConnected) {
        Serial.println("\n[BT] *** DISCONNECTED ***");
        Serial.print("[BT] Disconnection time: ");
        Serial.print(millis());
        Serial.println(" ms");
        Serial.print("[BT] Session samples sent: ");
        Serial.println(totalSamplesSent);
        Serial.print("[BT] Session batches sent: ");
        Serial.println(totalBatchesSent);
        Serial.println();
    }

    lastConnected = nowConnected;

    // EEG sample generation at 100 Hz
    if (currentMode == MODE_EEG && nowConnected) {
        unsigned long now = millis();
        
        // Generate sample every 10ms
        if (now - lastEEGGenerate >= EEG_INTERVAL_MS) {
            lastEEGGenerate += EEG_INTERVAL_MS;  // Fixed timing
            
            // Resync if fell too far behind
            if (now - lastEEGGenerate > EEG_INTERVAL_MS * 5) {
                Serial.println("[WARN] Timing resync - system fell behind");
                lastEEGGenerate = now;
            }

            // Skip initial frames after connection
            if (justConnected) {
                static uint8_t drop = 0;
                drop++;
                if (drop >= 2) {
                    justConnected = false;
                    drop = 0;
                    Serial.println("[EEG] Starting normal generation\n");
                }
                return;
            }

            // Generate and queue sample
            generateAndQueueSample();
        }
        
        // Send batch when full OR timeout (to avoid stalling)
        if ((queueCount >= BATCH_SIZE) || 
            (queueCount > 0 && now - lastBatchSend >= BATCH_TIMEOUT_MS)) {
            
#if DEBUG_ENABLED
            if (queueCount < BATCH_SIZE) {
                Serial.print("[INFO] Sending partial batch (");
                Serial.print(queueCount);
                Serial.println(" samples) due to timeout");
            }
#endif
            
            sendBatchedPacket();
        }
    }
    
    // Print timing statistics
    printTimingStats();
}