// /*
//  * main_debug.cpp - EEG Dummy Streaming over Bluetooth (Robust Reconnect) - DEBUG VERSION
//  * STM32F401 + JDY-23
//  * Author: Ahmad Taufiq
//  * Modified: Added detailed packet debugging
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
// #define EEG_INTERVAL_MS    5   // 100 Hz

// // ===== PACKET MARKER =====
// #define EEG_HEADER 0xAA
// #define EEG_FOOTER 0x55
// #define EEG_SYNC   0xF0   // special sync packet

// // ===== DEBUG CONFIG =====
// #define DEBUG_ENABLED      1    // Set 0 to disable all debug output
// #define DEBUG_SHOW_HEX     1    // Show packet in HEX format
// #define DEBUG_SHOW_VALUES  1    // Show EEG channel values
// #define DEBUG_SHOW_RAW     0    // Show complete raw packet bytes
// #define DEBUG_INTERVAL     1   // Show every N packets (1 = all packets)

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

// // ===== CRC8 (XOR SIMPLE) =====
// uint8_t crc8(uint8_t *data, uint8_t len)
// {
//     uint8_t crc = 0;
//     for (uint8_t i = 0; i < len; i++)
//         crc ^= data[i];
//     return crc;
// }

// // ===== DEBUG: PRINT PACKET INFO =====
// void debugPrintPacket(uint8_t *packet, uint8_t seq, int16_t *eegValues)
// {
// #if DEBUG_ENABLED
//     // Only print every DEBUG_INTERVAL packets
//     static uint32_t debugCounter = 0;
//     debugCounter++;
    
//     if (debugCounter % DEBUG_INTERVAL != 0) return;
    
//     Serial.println("========================================");
//     Serial.print("Packet #");
//     Serial.print(totalPacketsSent);
//     Serial.print(" | Seq: ");
//     Serial.print(seq);
//     Serial.print(" | Time: ");
//     Serial.print(millis());
//     Serial.println(" ms");
//     Serial.println("----------------------------------------");
    
// #if DEBUG_SHOW_HEX
//     // Header & Seq
//     Serial.print("Header: 0x");
//     Serial.print(packet[0], HEX);
//     Serial.print(" | Seq: 0x");
//     Serial.println(packet[1], HEX);
// #endif

// #if DEBUG_SHOW_VALUES
//     // EEG Channel Values
//     Serial.println("EEG Channels (16):");
//     for (int i = 0; i < EEG_CHANNELS; i++) {
//         Serial.print("  CH");
//         if (i < 10) Serial.print("0");
//         Serial.print(i);
//         Serial.print(": ");
        
//         // Show signed value with proper formatting
//         if (eegValues[i] >= 0) Serial.print(" ");
//         Serial.print(eegValues[i]);
//         Serial.print(" (0x");
        
//         // Show hex value
//         uint16_t hexVal = (uint16_t)eegValues[i];
//         if (hexVal < 0x1000) Serial.print("0");
//         if (hexVal < 0x100) Serial.print("0");
//         if (hexVal < 0x10) Serial.print("0");
//         Serial.print(hexVal, HEX);
//         Serial.print(")");
        
//         // New line every 4 channels for readability
//         if ((i + 1) % 4 == 0) {
//             Serial.println();
//         } else {
//             Serial.print(" | ");
//         }
//     }
//     if (EEG_CHANNELS % 4 != 0) Serial.println();
// #endif

// #if DEBUG_SHOW_HEX
//     // Footer & CRC
//     Serial.print("CRC: 0x");
//     Serial.print(packet[34], HEX);
//     Serial.print(" | Footer: 0x");
//     Serial.println(packet[35], HEX);
// #endif

// #if DEBUG_SHOW_RAW
//     // Complete raw packet
//     Serial.println("Raw Packet (36 bytes):");
//     for (int i = 0; i < EEG_PACKET_SIZE; i++) {
//         if (packet[i] < 0x10) Serial.print("0");
//         Serial.print(packet[i], HEX);
//         Serial.print(" ");
        
//         if ((i + 1) % 16 == 0) Serial.println();
//     }
//     if (EEG_PACKET_SIZE % 16 != 0) Serial.println();
// #endif

//     Serial.println("========================================\n");
// #endif
// }

// // ===== DEBUG: PRINT SYNC PACKET =====
// void debugPrintSyncPacket()
// {
// #if DEBUG_ENABLED
//     Serial.println("========================================");
//     Serial.println(">>> SYNC PACKET SENT <<<");
//     Serial.print("Time: ");
//     Serial.print(millis());
//     Serial.println(" ms");
//     Serial.print("Header: 0x");
//     Serial.print(eegPacket[0], HEX);
//     Serial.print(" | Sync: 0x");
//     Serial.print(eegPacket[1], HEX);
//     Serial.print(" | CRC: 0x");
//     Serial.print(eegPacket[34], HEX);
//     Serial.print(" | Footer: 0x");
//     Serial.println(eegPacket[35], HEX);
//     Serial.println("========================================\n");
// #endif
// }

// // ===== SEND SYNC PACKET =====
// void sendSyncPacket()
// {
//     memset(eegPacket, 0, EEG_PACKET_SIZE);

//     eegPacket[0]  = EEG_HEADER;
//     eegPacket[1]  = EEG_SYNC;   // special seq
//     eegPacket[34] = crc8(eegPacket, 34);
//     eegPacket[35] = EEG_FOOTER;

//     bt.send(eegPacket, EEG_PACKET_SIZE);

//     Serial.println("[BT] SYNC packet sent");
//     debugPrintSyncPacket();
// }

// // ===== RESET EEG STATE =====
// void resetEEGState()
// {
//     eegSeq = 0;
//     totalPacketsSent = 0;
//     lastEEGSend = millis();
//     eegDummyReset();
//     justConnected = true;

//     Serial.println("[EEG] State reset");
// }

// // ===== SEND EEG PACKET =====
// void sendEEGPacket()
// {
//     if (!bt.isConnected()) return;

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

//     // Debug print packet info
//     debugPrintPacket(eegPacket, eegSeq, eegInt);

//     eegSeq++;
//     totalPacketsSent++;
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
//     else if (data == "DEBUG_ON") {
//         // Command untuk enable debug via Bluetooth
//         bt.println("OK:DEBUG_ON");
//         Serial.println("[DEBUG] Debug output enabled");
//     }
//     else if (data == "DEBUG_OFF") {
//         bt.println("OK:DEBUG_OFF");
//         Serial.println("[DEBUG] Debug output disabled");
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
//     Serial.println("  STM32 EEG Bluetooth - DEBUG MODE");
//     Serial.println("========================================");
//     Serial.println("Debug Configuration:");
//     Serial.print("  - HEX Display: ");
//     Serial.println(DEBUG_SHOW_HEX ? "ON" : "OFF");
//     Serial.print("  - Values Display: ");
//     Serial.println(DEBUG_SHOW_VALUES ? "ON" : "OFF");
//     Serial.print("  - Raw Packet: ");
//     Serial.println(DEBUG_SHOW_RAW ? "ON" : "OFF");
//     Serial.print("  - Debug Interval: Every ");
//     Serial.print(DEBUG_INTERVAL);
//     Serial.println(" packet(s)");
//     Serial.println("========================================\n");

//     bt.begin(115200);
//     bt.onDataReceived(onBluetoothData);
//     bt.setAutoEcho(false);
    
//     Serial.println("[BT] Bluetooth initialized");
// }

// // ===== LOOP =====
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
//         sendSyncPacket();   // ⭐ penting
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

//     // EEG streaming
//     if (currentMode == MODE_EEG && nowConnected) {
//         unsigned long now = millis();
//         if (now - lastEEGSend >= EEG_INTERVAL_MS) {
//             lastEEGSend = now;

//             // Drop 2 frame pertama setelah connect
//             if (justConnected) {
//                 static uint8_t drop = 0;
//                 drop++;
//                 Serial.print("[EEG] Dropping initial frame ");
//                 Serial.print(drop);
//                 Serial.println("/2");
                
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
// }