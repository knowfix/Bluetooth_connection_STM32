// /*
//  * Simple 256 Bytes Transfer Test
//  * Test apakah 256 bytes bisa dikirim sekaligus tanpa data loss
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

// // ===== TEST DATA =====
// uint8_t testData[256];
// uint16_t testCounter = 0;

// // ===== CALCULATE CHECKSUM =====
// uint16_t calculateChecksum(uint8_t* data, uint16_t size) {
//     uint16_t checksum = 0;
//     for (uint16_t i = 0; i < size; i++) {
//         checksum += data[i];  // Simple sum
//     }
//     return checksum;
// } 

// // ===== GENERATE TEST DATA =====
// void generateTestData() {
//     // Pattern: 0, 1, 2, ..., 255
//     for (uint16_t i = 0; i < 256; i++) {
//         testData[i] = i;
//     }
//     Serial.println("[SETUP] Test data: 0-255 pattern");
// } 

// // ===== SEND 256 BYTES =====
// void send256Bytes() {
//     if (!bt.isConnected()) {
//         Serial.println("[ERROR] Not connected!");
//         return;
//     }
    
//     Serial.println("\n========================================");
//     Serial.println("  SENDING 256 BYTES TEST");
//     Serial.println("========================================");
    
//     testCounter++;
    
//     // Calculate checksum
//     uint16_t checksum = calculateChecksum(testData, 256);
    
//     // Send header with test info
//     bt.println(">>> START_256_TEST <<<");
//     bt.println("Test #" + String(testCounter));
//     bt.println("Data size: 256 bytes");
//     bt.println("Checksum: " + String(checksum));
//     bt.println("Pattern: 0-255 sequence");
//     bt.println("--- DATA START ---");
    
//     delay(100);  // Give receiver time to prepare
    
//     // Send 256 bytes as HEX string
//     Serial.println("[SENDING] Transmitting data...");
//     unsigned long startTime = millis();
    
//     String hexData = "";
//     for (uint16_t i = 0; i < 256; i++) {
//         if (testData[i] < 16) {
//             hexData += "0";  // Padding
//         }
//         hexData += String(testData[i], HEX);
        
//         // Send every 32 bytes (64 hex chars) to avoid buffer overflow
//         if ((i + 1) % 32 == 0 || i == 255) {
//             bt.send(hexData);
//             hexData = "";
//             delay(10);  // Small delay between chunks
//         }
//     }
    
//     bt.println("");  // Newline after data
    
//     unsigned long elapsed = millis() - startTime;
    
//     // Send footer
//     bt.println("--- DATA END ---");
//     bt.println("Checksum verify: " + String(checksum));
//     bt.println(">>> END_256_TEST <<<");
    
//     Serial.println("[SENT] Transfer complete!");
//     Serial.print("[TIME] ");
//     Serial.print(elapsed);
//     Serial.println(" ms");
//     Serial.print("[SPEED] ");
//     Serial.print((256.0 * 1000.0) / elapsed);
//     Serial.println(" bytes/sec");
//     Serial.println("========================================\n");
    
//     // Instructions
//     Serial.println("VERIFICATION INSTRUCTIONS:");
//     Serial.println("1. Check HP - should see 512 hex characters");
//     Serial.println("2. Count characters (should be exactly 512)");
//     Serial.println("3. Verify checksum matches");
//     Serial.println("   Expected checksum: " + String(checksum));
//     Serial.println("4. Verify pattern: 00 01 02 03 ... FE FF");
//     Serial.println("");
// }

// // ===== SEND AS DECIMAL (ALTERNATIVE) =====
// void send256BytesDecimal() {
//     if (!bt.isConnected()) {
//         Serial.println("[ERROR] Not connected!");
//         return;
//     }
    
//     Serial.println("\n========================================");
//     Serial.println("  SENDING 256 BYTES (DECIMAL FORMAT)");
//     Serial.println("========================================");
    
//     testCounter++;
//     uint16_t checksum = calculateChecksum(testData, 256);
    
//     bt.println(">>> START_256_TEST_DEC <<<");
//     bt.println("Test #" + String(testCounter));
//     bt.println("Checksum: " + String(checksum));
//     bt.println("--- DATA START ---");
    
//     delay(100);
    
//     unsigned long startTime = millis();
    
//     // Send as decimal numbers separated by space
//     for (uint16_t i = 0; i < 256; i++) {
//         bt.send(String(testData[i]));
//         bt.send(" ");
        
//         // Line break every 16 numbers for readability
//         if ((i + 1) % 16 == 0) {
//             bt.println("");
//             delay(5);
//         }
//     }
    
//     unsigned long elapsed = millis() - startTime;
    
//     bt.println("--- DATA END ---");
//     bt.println(">>> END_256_TEST_DEC <<<");
    
//     Serial.println("[SENT] Decimal transfer complete!");
//     Serial.print("[TIME] ");
//     Serial.print(elapsed);
//     Serial.println(" ms");
//     Serial.println("========================================\n");
// }

// // ===== SEND RAW BYTES (RISKY) =====
// void send256BytesRaw() {
//     if (!bt.isConnected()) {
//         Serial.println("[ERROR] Not connected!");
//         return;
//     }
    
//     Serial.println("\n========================================");
//     Serial.println("  SENDING 256 BYTES (RAW BINARY)");
//     Serial.println("========================================");
//     Serial.println("[WARNING] This may corrupt if special chars present!");
    
//     testCounter++;
    
//     bt.println(">>> START_RAW <<<");
//     delay(100);
    
//     unsigned long startTime = millis();
    
//     // Send raw bytes (DANGEROUS!)
//     for (uint16_t i = 0; i < 256; i++) {
//         bt.send(String((char)testData[i]));
//     }
    
//     unsigned long elapsed = millis() - startTime;
    
//     bt.println("");
//     bt.println(">>> END_RAW <<<");
    
//     Serial.println("[SENT] Raw transfer complete!");
//     Serial.print("[TIME] ");
//     Serial.print(elapsed);
//     Serial.println(" ms");
//     Serial.println("[NOTE] Check if all bytes received correctly");
//     Serial.println("========================================\n");
// }

// // ===== CALLBACK =====
// void onBluetoothData(String data) {
//     Serial.print("[BT->USB] ");
//     Serial.println(data);
    
//     if (data.equalsIgnoreCase("SEND_HEX")) {
//         send256Bytes();
//     }
//     else if (data.equalsIgnoreCase("SEND_DEC")) {
//         send256BytesDecimal();
//     }
//     else if (data.equalsIgnoreCase("SEND_RAW")) {
//         send256BytesRaw();
//     }
//     else if (data.equalsIgnoreCase("STATUS")) {
//         bt.println("Connected: " + String(bt.isConnected() ? "Yes" : "No"));
//         bt.println("Test count: " + String(testCounter));
//     }
// }

// // ===== SETUP =====
// void setup() {
//     // Initialize USB CDC
//     Serial.begin(115200);
//     unsigned long t0 = millis();
//     while (!Serial && millis() - t0 < 3000) delay(10);
    
//     Serial.println("\n========================================");
//     Serial.println("  256 BYTES TRANSFER TEST");
//     Serial.println("  Baudrate: 115200");
//     Serial.println("========================================");
    
//     // Initialize Bluetooth with 115200
//     bt.begin(115200);  // ← UPDATED BAUDRATE!
//     bt.onDataReceived(onBluetoothData);
//     bt.setAutoEcho(false);  // Disable auto echo
    
//     // Generate test data
//     generateTestData();
    
//     Serial.println("\n>> System Ready!");
//     Serial.println("\n>> COMMANDS:");
//     Serial.println("   SEND_HEX  - Send as hex string (recommended)");
//     Serial.println("   SEND_DEC  - Send as decimal numbers");
//     Serial.println("   SEND_RAW  - Send as raw bytes (risky)");
//     Serial.println("   STATUS    - Show status");
//     Serial.println("\n>> Type command in Serial Monitor or HP");
//     Serial.println("========================================\n");
// }

// // ===== LOOP =====
// void loop() {
//     bt.update();
    
//     // Handle USB Serial commands
//     if (Serial.available()) {
//         String cmd = Serial.readStringUntil('\n');
//         cmd.trim();
        
//         if (cmd.equalsIgnoreCase("SEND_HEX") || cmd.equalsIgnoreCase("SEND")) {
//             send256Bytes();
//         }
//         else if (cmd.equalsIgnoreCase("SEND_DEC")) {
//             send256BytesDecimal();
//         }
//         else if (cmd.equalsIgnoreCase("SEND_RAW")) {
//             send256BytesRaw();
//         }
//         else if (cmd.equalsIgnoreCase("STATUS")) {
//             Serial.println("Bluetooth: " + String(bt.isConnected() ? "Connected" : "Disconnected"));
//             Serial.println("Tests sent: " + String(testCounter));
//         }
//         else if (cmd.equalsIgnoreCase("HELP")) {
//             Serial.println("\nAvailable commands:");
//             Serial.println("  SEND_HEX  - Hex format (00010203...)");
//             Serial.println("  SEND_DEC  - Decimal format (0 1 2 3...)");
//             Serial.println("  SEND_RAW  - Raw binary (risky!)");
//             Serial.println("  STATUS    - Show info");
//             Serial.println("  HELP      - This message\n");
//         }
//         else {
//             Serial.println("Unknown command. Type HELP for commands.");
//         }
//     }
// }