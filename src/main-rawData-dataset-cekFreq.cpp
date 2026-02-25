// /*
//  * main_optimized.cpp - EEG Streaming with Fixed 100Hz Timing
//  * STM32F401 + JDY-23
//  * Author: Ahmad Taufiq
//  * Modified: Fixed timing issues for true 100Hz
//  */

// #include <Arduino.h>
// #include "Bluetooth.h"
// #include "eeg_dummy.h"

// // ===== PIN CONFIG =====
// #define PIN_BT_RX      PB7
// #define PIN_BT_TX      PB6
// #define PIN_BT_PWR     PA8
// #define PIN_BT_STAT    PB10
// #define PIN_BT_RST     PC9
// #define PIN_BT_LED     PC12

// // ===== EEG CONFIG =====
// #define EEG_CHANNELS       16
// #define EEG_PACKET_SIZE    36
// #define EEG_INTERVAL_MS    5   // 100 Hz target

// // ===== PACKET MARKER =====
// #define EEG_HEADER 0xAA
// #define EEG_FOOTER 0x55
// #define EEG_SYNC   0xF0

// // ===== DEBUG CONFIG =====
// #define DEBUG_ENABLED      1    // Set 0 to disable all debug
// #define DEBUG_SHOW_HEX     0    // ✅ Disabled for performance
// #define DEBUG_SHOW_VALUES  0    // ✅ Disabled for performance
// #define DEBUG_SHOW_RAW     0    // ✅ Disabled for performance
// #define DEBUG_INTERVAL     100  // ✅ Only every 100 packets
// #define DEBUG_TIMING       1    // ✅ Show timing statistics

// // ===== MODE =====
// enum SystemMode {
//     MODE_TEXT,
//     MODE_EEG
// };

// SystemMode currentMode = MODE_TEXT;

// // ===== BLUETOOTH =====
// BluetoothJDY23 bt(
//     PIN_BT_RX, PIN_BT_TX,
//     PIN_BT_PWR, PIN_BT_STAT,
//     PIN_BT_RST, PIN_BT_LED
// );

// // ===== EEG STATE =====
// uint8_t  eegPacket[EEG_PACKET_SIZE];
// uint8_t  eegSeq = 0;
// uint32_t totalPacketsSent = 0;
// unsigned long lastEEGSend = 0;
// bool justConnected = true;

// // ===== TIMING STATS =====
// unsigned long lastStatsTime = 0;
// uint32_t lastPacketCount = 0;
// unsigned long totalProcessingTime = 0;
// uint32_t processingCount = 0;

// // ===== CRC8 =====
// uint8_t crc8(uint8_t *data, uint8_t len)
// {
//     uint8_t crc = 0;
//     for (uint8_t i = 0; i < len; i++)
//         crc ^= data[i];
//     return crc;
// }

// // ===== OPTIMIZED DEBUG (Single print, formatted) =====
// void debugPrintPacket(uint8_t seq, unsigned long processingTime)
// {
// #if DEBUG_ENABLED
//     static uint32_t debugCounter = 0;
//     debugCounter++;
    
//     if (debugCounter % DEBUG_INTERVAL != 0) return;
    
//     // Single formatted print instead of multiple Serial.print() calls
//     char buf[128];
//     snprintf(buf, sizeof(buf), 
//         "[PKT] #%lu | Seq:%d | Time:%lu ms | ProcTime:%.2f ms\n",
//         totalPacketsSent, seq, millis(), processingTime / 1000.0
//     );
//     Serial.print(buf);
// #endif
// }

// // ===== SYNC PACKET =====
// void sendSyncPacket()
// {
//     memset(eegPacket, 0, EEG_PACKET_SIZE);

//     eegPacket[0]  = EEG_HEADER;
//     eegPacket[1]  = EEG_SYNC;
//     eegPacket[34] = crc8(eegPacket, 34);
//     eegPacket[35] = EEG_FOOTER;

//     bt.send(eegPacket, EEG_PACKET_SIZE);

//     Serial.println("[BT] SYNC packet sent");
// }

// // ===== RESET EEG STATE =====
// void resetEEGState()
// {
//     eegSeq = 0;
//     totalPacketsSent = 0;
//     lastEEGSend = millis();
//     eegDummyReset();
//     justConnected = true;
    
//     // Reset timing stats
//     lastStatsTime = millis();
//     lastPacketCount = 0;
//     totalProcessingTime = 0;
//     processingCount = 0;

//     Serial.println("[EEG] State reset");
// }

// // ===== SEND EEG PACKET (Optimized) =====
// void sendEEGPacket()
// {
//     if (!bt.isConnected()) return;

//     unsigned long startTime = micros();

//     float   eegFloat[EEG_CHANNELS];
//     int16_t eegInt[EEG_CHANNELS];

//     eegDummyGenerate16(eegFloat);

//     eegPacket[0] = EEG_HEADER;
//     eegPacket[1] = eegSeq;

//     for (int i = 0; i < EEG_CHANNELS; i++) {
//         int32_t v = (int32_t)roundf(eegFloat[i]);
//         if (v > 32767) v = 32767;
//         if (v < -32768) v = -32768;

//         eegInt[i] = (int16_t)v;

//         int idx = 2 + i * 2;
//         eegPacket[idx]     = eegInt[i] & 0xFF;
//         eegPacket[idx + 1] = (eegInt[i] >> 8) & 0xFF;
//     }

//     eegPacket[34] = crc8(eegPacket, 34);
//     eegPacket[35] = EEG_FOOTER;

//     bt.send(eegPacket, EEG_PACKET_SIZE);

//     unsigned long endTime = micros();
//     unsigned long processingTime = endTime - startTime;
    
//     // Track timing
//     totalProcessingTime += processingTime;
//     processingCount++;

//     // Optimized debug
//     debugPrintPacket(eegSeq, processingTime);

//     eegSeq++;
//     totalPacketsSent++;
// }

// // ===== TIMING STATISTICS =====
// void printTimingStats()
// {
// #if DEBUG_TIMING
//     static unsigned long lastPrint = 0;
//     unsigned long now = millis();
    
//     // Print every 5 seconds
//     if (now - lastPrint >= 5000) {
//         uint32_t packetsInPeriod = totalPacketsSent - lastPacketCount;
//         float actualHz = packetsInPeriod / 5.0;
        
//         float avgProcessingMs = 0;
//         if (processingCount > 0) {
//             avgProcessingMs = (totalProcessingTime / (float)processingCount) / 1000.0;
//         }
        
//         Serial.println("\n========== TIMING STATS ==========");
//         Serial.print("Target Frequency: 200 Hz (20ms interval)\n");
//         Serial.print("Actual Frequency: ");
//         Serial.print(actualHz, 2);
//         Serial.print(" Hz");
        
//         if (actualHz < 195) {
//             Serial.println(" ⚠️ TOO SLOW");
//         } else if (actualHz > 205) {
//             Serial.println(" ⚠️ TOO FAST");
//         } else {
//             Serial.println(" ✅ OK");
//         }
        
//         Serial.print("Avg Processing Time: ");
//         Serial.print(avgProcessingMs, 2);
//         Serial.println(" ms");
        
//         Serial.print("Total Packets Sent: ");
//         Serial.println(totalPacketsSent);
        
//         Serial.println("==================================\n");
        
//         lastPrint = now;
//         lastPacketCount = totalPacketsSent;
        
//         // Reset processing stats
//         totalProcessingTime = 0;
//         processingCount = 0;
//     }
// #endif
// }

// // ===== HANDLE TEXT COMMAND =====
// void onBluetoothData(String data)
// {
//     data.trim();
//     data.toUpperCase();

//     Serial.print("[CMD] ");
//     Serial.println(data);

//     if (data == "EEG_ON" || data == "START") {
//         currentMode = MODE_EEG;
//         resetEEGState();
//         bt.println("OK:EEG_ON");
//         Serial.println("[MODE] EEG Streaming STARTED");
//     }
//     else if (data == "EEG_OFF" || data == "STOP") {
//         currentMode = MODE_TEXT;
//         resetEEGState();
//         bt.println("OK:EEG_OFF");
//         Serial.println("[MODE] EEG Streaming STOPPED");
//     }
//     else if (data == "RESET") {
//         currentMode = MODE_TEXT;
//         resetEEGState();
//         bt.println("OK:RESET");
//         Serial.println("[MODE] System RESET");
//     }
//     else if (data == "STATUS") {
//         bt.println("MODE:" + String(currentMode == MODE_EEG ? "EEG" : "TEXT"));
//         Serial.print("[STATUS] Current mode: ");
//         Serial.println(currentMode == MODE_EEG ? "EEG" : "TEXT");
//         Serial.print("[STATUS] Total packets sent: ");
//         Serial.println(totalPacketsSent);
//         Serial.print("[STATUS] Current sequence: ");
//         Serial.println(eegSeq);
//     }
//     else {
//         bt.println("ERR:UNKNOWN_CMD");
//         Serial.println("[CMD] Unknown command");
//     }
// }

// // ===== SETUP =====
// void setup()
// {
//     Serial.begin(115200);
//     delay(1000);

//     Serial.println("\n========================================");
//     Serial.println("  STM32 EEG Bluetooth - OPTIMIZED");
//     Serial.println("========================================");
//     Serial.println("Configuration:");
//     Serial.print("  - Target Frequency: 100 Hz (");
//     Serial.print(EEG_INTERVAL_MS);
//     Serial.println("ms interval)");
//     Serial.print("  - Debug Interval: Every ");
//     Serial.print(DEBUG_INTERVAL);
//     Serial.println(" packets");
//     Serial.print("  - Timing Stats: ");
//     Serial.println(DEBUG_TIMING ? "ENABLED (every 5s)" : "DISABLED");
//     Serial.println("========================================\n");

//     bt.begin(38400);
//     bt.onDataReceived(onBluetoothData);
//     bt.setAutoEcho(false);
    
//     Serial.println("[BT] Bluetooth initialized");
//     Serial.println("[INFO] For best performance:");
//     Serial.println("       - DEBUG_ENABLED = 0 (production)");
//     Serial.println("       - DEBUG_INTERVAL >= 100 (if debug needed)");
//     Serial.println("");
// }

// // ===== LOOP (With Fixed Timing) =====
// void loop()
// {
//     bt.update();

//     // Detect reconnect
//     static bool lastConnected = false;
//     bool nowConnected = bt.isConnected();

//     if (nowConnected && !lastConnected) {
//         Serial.println("\n[BT] *** CONNECTED ***");
//         Serial.print("[BT] Connection time: ");
//         Serial.print(millis());
//         Serial.println(" ms\n");
        
//         resetEEGState();
//         sendSyncPacket();
//     }
    
//     if (!nowConnected && lastConnected) {
//         Serial.println("\n[BT] *** DISCONNECTED ***");
//         Serial.print("[BT] Disconnection time: ");
//         Serial.print(millis());
//         Serial.println(" ms");
//         Serial.print("[BT] Total packets sent this session: ");
//         Serial.println(totalPacketsSent);
//         Serial.println();
//     }

//     lastConnected = nowConnected;

//     // EEG streaming with FIXED TIMING
//     if (currentMode == MODE_EEG && nowConnected) {
//         unsigned long now = millis();
        
//         if (now - lastEEGSend >= EEG_INTERVAL_MS) {
//             // ✅ CRITICAL FIX: Increment instead of assignment
//             // This prevents timing drift
//             lastEEGSend += EEG_INTERVAL_MS;
            
//             // Handle case where we fell behind (e.g., after long processing)
//             // Catch up by skipping to current time
//             if (now - lastEEGSend > EEG_INTERVAL_MS * 5) {
//                 Serial.println("[WARN] Timing fell behind, resynchronizing...");
//                 lastEEGSend = now;
//             }

//             // Drop 2 frames after connect
//             if (justConnected) {
//                 static uint8_t drop = 0;
//                 drop++;
                
//                 if (drop >= 2) {
//                     justConnected = false;
//                     drop = 0;
//                     Serial.println("[EEG] Starting normal transmission...\n");
//                 }
//                 return;
//             }

//             sendEEGPacket();
//         }
//     }
    
//     // Print timing statistics periodically
//     printTimingStats();
// }