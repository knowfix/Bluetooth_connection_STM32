// /*
//  * main.cpp - EEG Dummy Streaming over Bluetooth (ASCII)
//  * STM32F401 + JDY-23
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
// #define EEG_INTERVAL_MS  10   // 100 Hz (aman buat ASCII)
// #define EEG_FS 100.0f   // sampling rate (Hz)


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

// unsigned long lastEEGSend = 0;
// uint32_t eegSeq = 0;

// // ===== COMMAND HANDLER =====
// void onBluetoothData(String data)
// {
//     data.trim();

//     if (data.equalsIgnoreCase("EEG_ON")) {
//         currentMode = MODE_EEG;
//         Serial.println("[MODE] EEG STREAMING ON");
//     }
//     else if (data.equalsIgnoreCase("EEG_OFF")) {
//         currentMode = MODE_TEXT;
//         Serial.println("[MODE] EEG STREAMING OFF");
//     }
//     else if (data.equalsIgnoreCase("STATUS")) {
//         bt.println(currentMode == MODE_EEG ? "MODE: EEG" : "MODE: TEXT");
//     }
// }

// // ===== SEND ASCII EEG =====
// void sendEEGAscii()
// {
//     int16_t eeg[8];
//     eegDummyGenerate(eeg);

//     // Hitung waktu (detik)
//     float t = eegSeq / EEG_FS;

//     String line = "";
//     line += String(t, 6);   // 6 digit desimal

//     for (int i = 0; i < 8; i++) {
//         line += ",";
//         line += String(eeg[i]);
//     }

//     bt.println(line);
//     eegSeq++;
// }
// // ===== SETUP =====
// void setup()
// {
//     Serial.begin(115200);
//     delay(1000);

//     Serial.println("STM32 EEG ASCII STREAM");

//     bt.begin(115200);
//     bt.onDataReceived(onBluetoothData);
//     bt.setAutoEcho(false);

//     Serial.println("Commands:");
//     Serial.println("  EEG_ON");
//     Serial.println("  EEG_OFF");
//     Serial.println("  STATUS");
// }

// // ===== LOOP =====
// void loop()
// {
//     // WAJIB
//     bt.update();

//     if (currentMode == MODE_EEG) {
//         if (millis() - lastEEGSend >= EEG_INTERVAL_MS) {
//             lastEEGSend = millis();
//             sendEEGAscii();
//         }
//     }
// }
