// /*
//  * main.cpp - Reliable 256 Bytes Transfer
//  * Kirim 256 bytes secepat mungkin tanpa data loss
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

// // ===== TRANSFER CONFIGURATION =====
// #define CHUNK_SIZE 16        // Bytes per chunk (optimal untuk 115200) -> Default 32
// #define CHUNK_DELAY 2       // Delay between chunks (ms) - CRITICAL! -> Default 15

// // ===== CREATE BLUETOOTH INSTANCE =====
// BluetoothJDY23 bt(PIN_BT_RX, PIN_BT_TX, PIN_BT_PWR, 
//                   PIN_BT_STAT, PIN_BT_RST, PIN_BT_LED);

// // ===== GLOBAL VARIABLES =====
// bool autoSendEnabled = false;
// unsigned long lastAutoSend = 0;
// const unsigned long AUTO_SEND_INTERVAL = 500;
// uint16_t sendCounter = 0;

// // ===== 256 BYTES MESSAGE =====
// // Exactly 256 characters (including spaces and punctuation)
// const char message256[] = 
//     "Ini adalah pesan uji coba sepanjang dua ratus lima puluh enam byte untuk "
//     "memastikan bahwa pengiriman data melalui modul Bluetooth berjalan stabil "
//     "tanpa kehilangan karakter atau korupsi data selama transmisi berlangsung. "
//     "Pengujian tambahan dilakukan.XXXXXXX";

// // ===== SEND 256 BYTES WITH CHUNKING =====
// bool send256BytesFast(const char* data) {
//     if (!bt.isConnected()) {
//         Serial.println("[ERROR] Not connected!");
//         return false;
//     }
    
//     // Verify length
//     int dataLen = strlen(data);
//     if (dataLen != 256) {
//         Serial.print("[WARNING] Data length is ");
//         Serial.print(dataLen);
//         Serial.println(" bytes, not 256!");
//     }
    
//     Serial.println("\n========================================");
//     Serial.println("  SENDING 256 BYTES");
//     Serial.println("========================================");
//     Serial.print("Length: ");
//     Serial.println(dataLen);
    
//     unsigned long startTime = millis();
    
//     // Send START marker
//     bt.println(">>> START <<<");
//     delay(50);  // Give receiver time to prepare
    
//     // Send data in chunks
//     int totalChunks = (dataLen + CHUNK_SIZE - 1) / CHUNK_SIZE;
    
//     for (int i = 0; i < dataLen; i += CHUNK_SIZE) {
//         int chunkLen = min(CHUNK_SIZE, dataLen - i);
//         int chunkNum = (i / CHUNK_SIZE) + 1;
        
//         // Extract chunk
//         char chunk[CHUNK_SIZE + 1];
//         memcpy(chunk, data + i, chunkLen);
//         chunk[chunkLen] = '\0';
        
//         // Send chunk
//         bt.send(chunk);
        
//         // Progress indicator
//         Serial.print("[CHUNK ");
//         Serial.print(chunkNum);
//         Serial.print("/");
//         Serial.print(totalChunks);
//         Serial.print("] Sent ");
//         Serial.print(chunkLen);
//         Serial.println(" bytes");
        
//         // CRITICAL: Delay between chunks
//         delay(CHUNK_DELAY);
//     }
    
//     // Send END marker
//     bt.println("");  // Newline
//     bt.println(">>> END <<<");
    
//     unsigned long elapsed = millis() - startTime;
    
//     // Statistics
//     Serial.println("========================================");
//     Serial.println("✓ Transfer complete!");
//     Serial.print("Time: ");
//     Serial.print(elapsed);
//     Serial.println(" ms");
//     Serial.print("Speed: ");
//     Serial.print((dataLen * 1000.0) / elapsed);
//     Serial.println(" bytes/sec");
//     Serial.println("========================================\n");
    
//     return true;
// }

// // ===== SEND WITH AUTO-RETRY =====
// bool send256BytesWithRetry(const char* data, int maxRetries = 3) {
//     for (int retry = 0; retry < maxRetries; retry++) {
//         if (retry > 0) {
//             Serial.print("[RETRY] Attempt ");
//             Serial.print(retry + 1);
//             Serial.print("/");
//             Serial.println(maxRetries);
//             delay(100);
//         }
        
//         if (send256BytesFast(data)) {
//             return true;
//         }
//     }
    
//     Serial.println("[FAILED] All retry attempts exhausted!");
//     return false;
// }

// // ===== CALLBACK FUNCTION =====
// void onBluetoothData(String data) {
//     Serial.print("[BT->USB] ");
//     Serial.println(data);
    
//     // Commands
//     if (data.equalsIgnoreCase("SEND")) {
//         sendCounter++;
//         Serial.print("\n>>> Manual send #");
//         Serial.println(sendCounter);
//         send256BytesFast(message256);
//     }
//     else if (data.equalsIgnoreCase("AUTO_ON")) {
//         autoSendEnabled = true;
//         bt.println("Auto-send: ON (every 3s)");
//         Serial.println("[CMD] Auto-send ENABLED");
//     }
//     else if (data.equalsIgnoreCase("AUTO_OFF")) {
//         autoSendEnabled = false;
//         bt.println("Auto-send: OFF");
//         Serial.println("[CMD] Auto-send DISABLED");
//     }
//     else if (data.equalsIgnoreCase("STATUS")) {
//         bt.println("Connected: " + String(bt.isConnected() ? "Yes" : "No"));
//         bt.println("Auto-send: " + String(autoSendEnabled ? "ON" : "OFF"));
//         bt.println("Send count: " + String(sendCounter));
//     }
//     else if (data.equalsIgnoreCase("TEST")) {
//         // Quick test
//         bt.println("Testing 256 bytes transfer...");
//         send256BytesWithRetry(message256);
//     }
// }

// // ===== SETUP =====
// void setup() {
//     // Initialize USB CDC
//     Serial.begin(115200);
//     unsigned long t0 = millis();
//     while (!Serial && millis() - t0 < 3000) delay(10);
    
//     Serial.println("\n========================================");
//     Serial.println("  STM32F401 + JDY-23 Bluetooth");
//     Serial.println("  256 Bytes Fast Transfer");
//     Serial.println("========================================");
    
//     // Verify message length
//     int msgLen = strlen(message256);
//     Serial.print("Message length: ");
//     Serial.print(msgLen);
//     Serial.println(" bytes");
    
//     if (msgLen != 256) {
//         Serial.println("[WARNING] Message is not exactly 256 bytes!");
//     } else {
//         Serial.println("[OK] Message is exactly 256 bytes");
//     }
    
//     // Initialize Bluetooth
//     bt.begin(115200);
//     bt.onDataReceived(onBluetoothData);
//     bt.setAutoEcho(false);
    
//     Serial.println("\n========================================");
//     Serial.println("COMMANDS:");
//     Serial.println("  SEND      - Manual send 256 bytes");
//     Serial.println("  AUTO_ON   - Enable auto-send (3s)");
//     Serial.println("  AUTO_OFF  - Disable auto-send");
//     Serial.println("  STATUS    - Show status");
//     Serial.println("  TEST      - Test with retry");
//     Serial.println("========================================");
//     Serial.println("\n>> System Ready!");
//     Serial.println(">> Type 'SEND' to transfer 256 bytes\n");
// }

// // ===== LOOP =====
// void loop() {
//     // Update Bluetooth
//     bt.update();
    
//     // Handle USB Serial commands
//     if (Serial.available()) {
//         String cmd = Serial.readStringUntil('\n');
//         cmd.trim();
        
//         if (cmd.equalsIgnoreCase("SEND")) {
//             sendCounter++;
//             Serial.print("\n>>> Send #");
//             Serial.println(sendCounter);
//             send256BytesFast(message256);
//         }
//         else if (cmd.equalsIgnoreCase("AUTO_ON")) {
//             autoSendEnabled = true;
//             Serial.println("[CMD] Auto-send ENABLED");
//         }
//         else if (cmd.equalsIgnoreCase("AUTO_OFF")) {
//             autoSendEnabled = false;
//             Serial.println("[CMD] Auto-send DISABLED");
//         }
//         else if (cmd.equalsIgnoreCase("STATUS")) {
//             Serial.println("\n========================================");
//             Serial.println("STATUS:");
//             Serial.print("  Connection: ");
//             Serial.println(bt.isConnected() ? "Connected" : "Disconnected");
//             Serial.print("  Auto-send: ");
//             Serial.println(autoSendEnabled ? "ON" : "OFF");
//             Serial.print("  Send count: ");
//             Serial.println(sendCounter);
//             Serial.print("  Message length: ");
//             Serial.println(strlen(message256));
//             Serial.println("========================================\n");
//         }
//         else if (cmd.equalsIgnoreCase("TEST")) {
//             sendCounter++;
//             Serial.print("\n>>> Test send #");
//             Serial.println(sendCounter);
//             send256BytesWithRetry(message256);
//         }
//         else if (cmd.equalsIgnoreCase("HELP")) {
//             Serial.println("\nCOMMANDS:");
//             Serial.println("  SEND      - Send 256 bytes once");
//             Serial.println("  AUTO_ON   - Enable auto-send every 3s");
//             Serial.println("  AUTO_OFF  - Disable auto-send");
//             Serial.println("  STATUS    - Show current status");
//             Serial.println("  TEST      - Send with auto-retry");
//             Serial.println("  HELP      - Show this help\n");
//         }
//         else {
//             Serial.println("Unknown command. Type HELP for commands.");
//         }
//     }
    
//     // Auto-send (if enabled)
//     if (autoSendEnabled && millis() - lastAutoSend >= AUTO_SEND_INTERVAL) {
//         lastAutoSend = millis();
//         if (bt.isConnected()) {
//             sendCounter++;
//             Serial.print("\n>>> Auto-send #");
//             Serial.println(sendCounter);
//             send256BytesFast(message256);
//         }
//     }
// }