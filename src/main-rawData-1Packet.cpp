// /*
//  * main.cpp - EEG Dummy Streaming over Bluetooth (Single Packet)
//  * STM32F401 + JDY-23
//  * Author: Ahmad Taufiq
//  * Date: 2025
//  */

// #include <Arduino.h>
// #include "Bluetooth.h"
// #include "eeg_dummy.h"

// // ===== PIN CONFIGURATION =====
// #define PIN_BT_RX      PB7
// #define PIN_BT_TX      PB6
// #define PIN_BT_PWR     PA8
// #define PIN_BT_STAT    PB10
// #define PIN_BT_RST     PC9
// #define PIN_BT_LED     PC12

// // ===== EEG CONFIG =====
// #define EEG_CHANNELS       16
// #define EEG_PACKET_SIZE    36    // ✅ UBAH: Header(1) + SEQ(1) + 16ch×2(32) + CRC(1) + Footer(1)
// #define EEG_INTERVAL_MS    10    // 100 Hz (lebih stabil untuk packet besar)

// // ===== MODE =====
// enum SystemMode {
//     MODE_TEXT,
//     MODE_EEG
// };

// SystemMode currentMode = MODE_TEXT;

// // ===== BLUETOOTH INSTANCE =====
// BluetoothJDY23 bt(PIN_BT_RX, PIN_BT_TX, PIN_BT_PWR,
//                   PIN_BT_STAT, PIN_BT_RST, PIN_BT_LED);

// // ===== EEG VARIABLES =====
// uint8_t  eegPacket[EEG_PACKET_SIZE];
// uint8_t  eegSeq = 0;
// uint32_t totalPacketsSent = 0;
// unsigned long lastEEGSend = 0;

// // ===== CRC8 =====
// uint8_t crc8(uint8_t *data, uint8_t len)
// {
//     uint8_t crc = 0;
//     for (uint8_t i = 0; i < len; i++)
//         crc ^= data[i];
//     return crc;
// }

// // ===== SEND EEG PACKET =====
// void sendEEGPacket()
// {
//     if (!bt.isConnected()) return;

//     int16_t eeg[16];
//     eegDummyGenerate16(eeg);

//     // Packet structure (36 bytes):
//     // [0]     = 0xAA (header)
//     // [1]     = sequence number
//     // [2-33]  = 16 channels × 2 bytes (little endian)
//     // [34]    = CRC8
//     // [35]    = 0x55 (footer)

//     eegPacket[0] = 0xAA;
//     eegPacket[1] = eegSeq;

//     // Pack all 16 channels
//     for (int i = 0; i < 16; i++)
//     {
//         int16_t v = eeg[i];
//         int packetIndex = 2 + (i * 2);
        
//         // Little endian: low byte first, high byte second
//         eegPacket[packetIndex]     = (uint8_t)(v & 0xFF);
//         eegPacket[packetIndex + 1] = (uint8_t)((v >> 8) & 0xFF);
//     }

//     // CRC calculation (bytes 0-33)
//     eegPacket[34] = crc8(eegPacket, 34);
    
//     // Footer
//     eegPacket[35] = 0x55;

//     // ✅ DEBUGGING: Print packet setiap 50 sequence
//     if (eegSeq % 50 == 0) {
//         Serial.print("[PKT] SEQ=");
//         Serial.print(eegSeq);
//         Serial.print(" | EEG: ");
//         for (int i = 0; i < 16; i++) {
//             Serial.print(eeg[i]);
//             Serial.print(" ");
//         }
//         Serial.print("| CRC=");
//         Serial.println(eegPacket[34]);
//     }

//     // Send single packet
//     bt.send(eegPacket, EEG_PACKET_SIZE);

//     eegSeq++;
//     totalPacketsSent++;
// }

// // ===== HANDLE TEXT COMMAND =====
// void onBluetoothData(String data)
// {
//     data.trim();
//     String cmd = data;
//     cmd.toUpperCase();

//     Serial.print("[CMD] Received: ");
//     Serial.println(cmd);

//     if (cmd == "EEG_ON" || cmd == "START") {
//         currentMode = MODE_EEG;
//         eegSeq = 0;
//         totalPacketsSent = 0;
//         lastEEGSend = millis();
//         bt.println("OK:EEG_ON");
//         Serial.println("[MODE] ✓ EEG STREAMING ON");
//     }
//     else if (cmd == "EEG_OFF" || cmd == "STOP") {
//         currentMode = MODE_TEXT;
//         bt.println("OK:EEG_OFF");
//         Serial.println("[MODE] ✓ EEG STREAMING OFF");
//     }
//     else if (cmd == "STATUS") {
//         String status = "MODE:";
//         status += (currentMode == MODE_EEG) ? "EEG" : "TEXT";
//         status += " SEQ:";
//         status += eegSeq;
//         status += " TOTAL:";
//         status += totalPacketsSent;
//         bt.println(status);
//         Serial.println(status);
//     }
//     else {
//         bt.println("UNKNOWN:" + data);
//         Serial.println("[CMD] Unknown command");
//     }
// }

// // ===== SETUP =====
// void setup()
// {
//     Serial.begin(115200);
//     delay(1000);

//     Serial.println("\n======================================");
//     Serial.println(" STM32 + JDY-23 EEG SINGLE PACKET ");
//     Serial.println(" Packet Size: 36 bytes (16 channels) ");
//     Serial.println("======================================");

//     bt.begin(115200);
//     bt.onDataReceived(onBluetoothData);
//     bt.setAutoEcho(true);

//     Serial.println("Commands:");
//     Serial.println("  EEG_ON   → Start EEG streaming");
//     Serial.println("  EEG_OFF  → Stop EEG streaming");
//     Serial.println("  STATUS   → Show mode");
//     Serial.println("======================================\n");
// }

// // ===== LOOP =====
// void loop()
// {
//     bt.update();

//     if (currentMode == MODE_EEG && bt.isConnected()) {
//         unsigned long now = millis();
//         if (now - lastEEGSend >= EEG_INTERVAL_MS) {
//             lastEEGSend = now;
//             sendEEGPacket();
            
//             // Debug setiap 100 packets (1 detik @ 100Hz)
//             if (totalPacketsSent % 100 == 0) {
//                 Serial.print("[EEG] Total: ");
//                 Serial.print(totalPacketsSent);
//                 Serial.print(" | Seq: ");
//                 Serial.println(eegSeq);
//             }
//         }
//     }
// }