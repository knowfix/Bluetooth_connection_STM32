// /*
//  * main.cpp - EEG Dummy Streaming over Bluetooth
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
// #define EEG_PACKET_SIZE   20

// // #define EEG_PACKET_SIZE     20
// int16_t eeg[EEG_CHANNELS];

// #define EEG_INTERVAL_MS     4    // 250 Hz

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

//     for (uint8_t part = 0; part < 2; part++)
//     {
//         eegPacket[0] = 0xAA;
//         eegPacket[1] = eegSeq;
//         eegPacket[2] = part;   // 0 = CH1..8, 1 = CH9..16

//         for (int i = 0; i < 8; i++)
//         {
//             int idx = part * 8 + i;
//             int16_t v = eeg[idx];

//             eegPacket[3 + i*2] = v & 0xFF;
//             eegPacket[4 + i*2] = (v >> 8) & 0xFF;
//         }

//         eegPacket[18] = crc8(eegPacket, 18);
//         eegPacket[19] = 0x55;

//         bt.send(eegPacket, EEG_PACKET_SIZE);

//         delayMicroseconds(500);   // penting untuk BLE (hindari packet collision)
//     }

//     eegSeq++;
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
//         eegSeq = 0;  // Reset sequence number
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
//     Serial.println(" STM32 + JDY-23 EEG DUMMY STREAMING ");
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
//     // ALWAYS update to receive commands
//     bt.update();

//     // EEG streaming logic
//     if (currentMode == MODE_EEG && bt.isConnected()) {
//         unsigned long now = millis();
//         if (now - lastEEGSend >= EEG_INTERVAL_MS) {
//             lastEEGSend = now;
//             sendEEGPacket();
            
//             // Debug output every 250 packets (1 second at 250Hz)
//             if (eegSeq % 250 == 0) {
//                 Serial.print("[EEG] Streaming... Seq: ");
//                 Serial.println(eegSeq);
//             }
//         }
//     }
// }