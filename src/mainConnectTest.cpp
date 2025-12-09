// /*
//  * main.cpp
//  * Program utama untuk STM32F401 + JDY-23 Bluetooth
//  * Author: Ahmad Taufiq
//  * Date: 2025
//  */

// #include <Arduino.h>
// #include <Bluetooth.h>

// // ===== PIN CONFIGURATION =====
// #define PIN_BT_RX      PB7
// #define PIN_BT_TX      PB6
// #define PIN_BT_PWR     PA8
// #define PIN_BT_STAT    PB10
// #define PIN_BT_RST     PC9
// #define PIN_BT_LED     PC12

// // ===== CREATE BLUETOOTH INSTANCE =====
// BluetoothJDY23 bt(PIN_BT_RX, PIN_BT_TX, PIN_BT_PWR, 
//                   PIN_BT_STAT, PIN_BT_RST, PIN_BT_LED);

// // ===== GLOBAL VARIABLES =====
// unsigned long lastPing = 0;
// const unsigned long PING_INTERVAL = 3000; // 3 seconds

// // ===== CALLBACK FUNCTION =====
// // Fungsi ini dipanggil otomatis ketika ada data dari Bluetooth
// void onBluetoothData(String data) {
//     Serial.print("[BT->USB] ");
//     Serial.println(data);
    
//     // Process commands
//     if (data.equalsIgnoreCase("HELLO")) {
//         bt.println("Hi! I'm STM32F401");
//     }
//     else if (data.equalsIgnoreCase("STATUS")) {
//         bt.println(bt.isConnected() ? "Connected" : "Disconnected");
//     }
//     else if (data.equalsIgnoreCase("TIME")) {
//         bt.println("Uptime: " + String(millis() / 1000) + " seconds");
//     }
// }

// // ===== SETUP =====
// void setup() {
//     // Initialize USB CDC
//     Serial.begin(115200);
//     unsigned long t0 = millis();
//     while (!Serial && millis() - t0 < 3000) delay(10);
    
//     Serial.println("\n====================================");
//     Serial.println("  STM32F401 + JDY-23 Bluetooth");
//     Serial.println("  Modular Architecture");
//     Serial.println("====================================");
    
//     // Initialize Bluetooth
//     bt.begin(115200); // JDY-23 baudrate
    
//     // Set callback function
//     bt.onDataReceived(onBluetoothData);
    
//     // PENTING!!! Cara set baudrate BluetoothJDY23
//     // bt.println("AT+BAUD0"); 
    
//     // Optional: disable auto echo if you want manual control
//     // bt.setAutoEcho(false);
    
//     Serial.println("\n>> System Ready!");
//     Serial.println(">> Send data from phone or Serial Monitor\n");
// }

// // ===== LOOP =====
// void loop() {
//     // Update Bluetooth (check connection, handle incoming data)
//     bt.update();
    
//     // Handle commands from USB Serial
//     if (Serial.available()) {
//         String cmd = Serial.readStringUntil('\n');
//         cmd.trim();
        
//         if (cmd.equalsIgnoreCase("RST")) {
//             bt.reset();
//         }
//         else if (cmd.equalsIgnoreCase("STATUS")) {
//             Serial.print("Connection: ");
//             Serial.println(bt.isConnected() ? "Connected" : "Disconnected");
//         }
//         else {
//             // Send to Bluetooth
//             Serial.print("[USB->BT] ");
//             Serial.println(cmd);
//             bt.println(cmd);
//         }
//     }
    
//     // Periodic ping
//     if (millis() - lastPing >= PING_INTERVAL) {
//         lastPing = millis();
//         if (bt.isConnected()) {
//             bt.println("Ping from STM32!");
//             Serial.println("[PING] Sent to Bluetooth");
//         }
//     }
// }