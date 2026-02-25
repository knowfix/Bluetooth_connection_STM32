// /*
//  * Reed-Solomon FEC - Send Plain Text to HP
//  * Encode → Simulate Error → Decode → Send corrected text
//  */

// #include <Arduino.h>
// #include <Bluetooth.h>
// #include "RS-FEC.h"

// // ===== PIN CONFIGURATION =====
// #define PIN_BT_RX      PB7
// #define PIN_BT_TX      PB6
// #define PIN_BT_PWR     PA8 
// #define PIN_BT_STAT    PB10
// #define PIN_BT_RST     PC9
// #define PIN_BT_LED     PC12 

// // ===== FEC CONFIGURATION =====
// #define DATA_LENGTH 223
// #define ECC_LENGTH  32
// #define ENCODED_LENGTH (DATA_LENGTH + ECC_LENGTH)

// // ===== CREATE INSTANCES =====
// BluetoothJDY23 bt(PIN_BT_RX, PIN_BT_TX, PIN_BT_PWR, 
//                   PIN_BT_STAT, PIN_BT_RST, PIN_BT_LED);

// RS::ReedSolomon<DATA_LENGTH, ECC_LENGTH> rsCodec;

// // ===== TEST MESSAGE =====
// const char testMessage[DATA_LENGTH + 1] = 
//     "Ini adalah pesan uji coba sepanjang dua ratus lima puluh enam byte untuk "
//     "memastikan bahwa pengiriman data melalui modul Bluetooth berjalan stabil "
//     "tanpa kehilangan karakter atau korupsi data selama transmisi berlangsung.";

// uint16_t sendCounter = 0;

// // ===== SEND WITH FEC - KIRIM TEXT YANG SUDAH DIKOREKSI =====
// void sendWithFEC(bool simulateError = false) {
//     if (!bt.isConnected()) {
//         Serial.println("[ERROR] Not connected!");
//         return;
//     }
    
//     Serial.println("\n========================================");
//     Serial.println("  SEND WITH REED-SOLOMON FEC");
//     Serial.println("========================================");
    
//     sendCounter++;
//     unsigned long startTime = millis();
    
//     // Prepare data
//     uint8_t originalData[DATA_LENGTH];
//     memcpy(originalData, testMessage, DATA_LENGTH);
    
//     Serial.println("[STEP 1] Original message prepared");
//     Serial.print("  Length: ");
//     Serial.print(DATA_LENGTH);
//     Serial.println(" bytes");
    
//     // Encode with Reed-Solomon
//     uint8_t encodedData[ENCODED_LENGTH];
//     rsCodec.Encode(originalData, encodedData);
    
//     Serial.println("[STEP 2] Encoded with Reed-Solomon");
//     Serial.print("  Encoded length: ");
//     Serial.print(ENCODED_LENGTH);
//     Serial.print(" bytes (");
//     Serial.print(DATA_LENGTH);
//     Serial.print(" data + ");
//     Serial.print(ECC_LENGTH);
//     Serial.println(" parity)");
    
//     // Simulate transmission errors (optional)
//     int numErrors = 0;
//     if (simulateError) {
//         numErrors = random(5, 16);  // Random 5-15 errors
//         Serial.print("[STEP 3] Simulating ");
//         Serial.print(numErrors);
//         Serial.println(" transmission errors...");
        
//         for (int i = 0; i < numErrors; i++) {
//             int pos = random(0, ENCODED_LENGTH);
//             uint8_t oldVal = encodedData[pos];
//             encodedData[pos] = random(0, 256);
            
//             Serial.print("  Error ");
//             Serial.print(i + 1);
//             Serial.print(" at position ");
//             Serial.print(pos);
//             Serial.print(": 0x");
//             if (oldVal < 16) Serial.print("0");
//             Serial.print(oldVal, HEX);
//             Serial.print(" → 0x");
//             if (encodedData[pos] < 16) Serial.print("0");
//             Serial.println(encodedData[pos], HEX);
//         }
//     } else {
//         Serial.println("[STEP 3] No errors simulated (clean transfer)");
//     }
    
//     // Decode to get corrected data
//     uint8_t correctedData[DATA_LENGTH];
//     int decodeResult = rsCodec.Decode(encodedData, correctedData);
    
//     if (decodeResult != 0) {
//         Serial.println("[STEP 4] ✗ DECODE FAILED!");
//         Serial.println("  Too many errors to correct!");
//         bt.println(">>> ERROR: Too many transmission errors! <<<");
//         return;
//     }
    
//     Serial.println("[STEP 4] ✓ Decoded successfully!");
    
//     // Verify correction
//     bool perfect = (memcmp(originalData, correctedData, DATA_LENGTH) == 0);
//     if (perfect) {
//         if (simulateError) {
//             Serial.print("  ✓ Corrected ");
//             Serial.print(numErrors);
//             Serial.println(" errors successfully!");
//         } else {
//             Serial.println("  ✓ Data verified (no errors)");
//         }
//     } else {
//         Serial.println("  ✗ Warning: Some errors remain!");
//     }
    
//     // Send corrected text to HP
//     Serial.println("[STEP 5] Sending corrected text to HP...");
    
//     bt.println("========================================");
//     bt.send(">>> TRANSFER #");
//     bt.send(String(sendCounter));
//     bt.println(" <<<");
//     bt.println("========================================");
    
//     if (simulateError) {
//         bt.send("Errors simulated: ");
//         bt.println(String(numErrors));
//         bt.send("Errors corrected: ");
//         bt.println(String(perfect ? numErrors : numErrors - 1));
//         bt.println("Status: " + String(perfect ? "PERFECT" : "PARTIAL"));
//         bt.println("----------------------------------------");
//     }
    
//     // Send the corrected message as plain text
//     bt.println((char*)correctedData);
    
//     bt.println("========================================");
    
//     unsigned long elapsed = millis() - startTime;
    
//     Serial.println("========================================");
//     Serial.println("✓ TRANSFER COMPLETE!");
//     Serial.print("Time: ");
//     Serial.print(elapsed);
//     Serial.println(" ms");
//     Serial.print("Status: ");
//     Serial.println(perfect ? "PERFECT" : "PARTIAL");
//     Serial.println("========================================\n");
// }

// // ===== SEND WITHOUT FEC (COMPARISON) =====
// void sendWithoutFEC() {
//     if (!bt.isConnected()) {
//         Serial.println("[ERROR] Not connected!");
//         return;
//     }
    
//     Serial.println("\n========================================");
//     Serial.println("  SEND WITHOUT FEC (BASELINE)");
//     Serial.println("========================================");
    
//     sendCounter++;
//     unsigned long startTime = millis();
    
//     bt.println("========================================");
//     bt.send(">>> TRANSFER #");
//     bt.send(String(sendCounter));
//     bt.println(" (NO FEC) <<<");
//     bt.println("========================================");
//     bt.println((char*)testMessage);
//     bt.println("========================================");
    
//     unsigned long elapsed = millis() - startTime;
    
//     Serial.println("✓ TRANSFER COMPLETE!");
//     Serial.print("Time: ");
//     Serial.print(elapsed);
//     Serial.println(" ms");
//     Serial.println("========================================\n");
// }

// // ===== TEST FEC LOCALLY =====
// void testFECLocal() {
//     Serial.println("\n========================================");
//     Serial.println("  LOCAL FEC TEST");
//     Serial.println("========================================");
    
//     uint8_t originalData[DATA_LENGTH];
//     memcpy(originalData, testMessage, DATA_LENGTH);
    
//     Serial.println("[1] Original message:");
//     Serial.print("  First 80 chars: \"");
//     for (int i = 0; i < 80; i++) Serial.print((char)originalData[i]);
//     Serial.println("...\"");
    
//     // Encode
//     uint8_t encodedData[ENCODED_LENGTH];
//     rsCodec.Encode(originalData, encodedData);
//     Serial.println("[2] ✓ Encoded with Reed-Solomon");
    
//     // Simulate errors
//     int numErrors = 12;
//     Serial.print("[3] Injecting ");
//     Serial.print(numErrors);
//     Serial.println(" random errors...");
    
//     for (int i = 0; i < numErrors; i++) {
//         int pos = random(0, ENCODED_LENGTH);
//         uint8_t oldVal = encodedData[pos];
//         encodedData[pos] = random(0, 256);
        
//         Serial.print("  Error ");
//         Serial.print(i + 1);
//         Serial.print(": pos=");
//         Serial.print(pos);
//         Serial.print(", 0x");
//         if (oldVal < 16) Serial.print("0");
//         Serial.print(oldVal, HEX);
//         Serial.print("→0x");
//         if (encodedData[pos] < 16) Serial.print("0");
//         Serial.println(encodedData[pos], HEX);
//     }
    
//     // Decode
//     Serial.println("[4] Decoding and correcting errors...");
//     uint8_t decodedData[DATA_LENGTH];
//     int result = rsCodec.Decode(encodedData, decodedData);
    
//     Serial.println("========================================");
//     if (result == 0) {
//         bool success = (memcmp(originalData, decodedData, DATA_LENGTH) == 0);
        
//         if (success) {
//             Serial.println("✓✓✓ TEST PASSED! ✓✓✓");
//             Serial.print("✓ Corrected all ");
//             Serial.print(numErrors);
//             Serial.println(" errors!");
//             Serial.println("✓ Message recovered perfectly!");
//             Serial.println("\n[5] Recovered message:");
//             Serial.print("  \"");
//             for (int i = 0; i < 80; i++) Serial.print((char)decodedData[i]);
//             Serial.println("...\"");
//         } else {
//             Serial.println("✗ TEST FAILED!");
//             Serial.println("Some bytes still incorrect.");
//         }
//     } else {
//         Serial.println("✗ DECODE FAILED!");
//         Serial.println("Too many errors to correct.");
//     }
//     Serial.println("========================================\n");
// }

// // ===== CALLBACK =====
// void onBluetoothData(String data) {
//     data.trim();
//     Serial.print("[BT→] ");
//     Serial.println(data);
    
//     if (data.equalsIgnoreCase("SEND")) {
//         sendWithFEC(false);  // Clean transfer
//     }
//     else if (data.equalsIgnoreCase("SEND_ERROR")) {
//         sendWithFEC(true);   // With simulated errors
//     }
//     else if (data.equalsIgnoreCase("SEND_PLAIN")) {
//         sendWithoutFEC();
//     }
//     else if (data.equalsIgnoreCase("TEST")) {
//         testFECLocal();
//     }
//     else if (data.equalsIgnoreCase("STATUS")) {
//         bt.println("=== STATUS ===");
//         bt.println("Connected: Yes");
//         bt.send("Transfers: ");
//         bt.println(String(sendCounter));
//         bt.send("Data size: ");
//         bt.send(String(DATA_LENGTH));
//         bt.println(" bytes");
//         bt.send("ECC: ");
//         bt.send(String(ECC_LENGTH));
//         bt.println(" bytes");
//         bt.send("Max correction: ");
//         bt.send(String(ECC_LENGTH / 2));
//         bt.println(" errors");
//     }
// }

// // ===== SETUP =====
// void setup() {
//     Serial.begin(115200);
//     while (!Serial && millis() < 3000);
    
//     Serial.println("\n========================================");
//     Serial.println("  REED-SOLOMON FEC DEMO");
//     Serial.println("  STM32F401 + JDY-23");
//     Serial.println("========================================");
    
//     int msgLen = strlen(testMessage);
//     Serial.print("Message length: ");
//     Serial.print(msgLen);
//     Serial.print(" / ");
//     Serial.print(DATA_LENGTH);
//     Serial.println(msgLen == DATA_LENGTH ? " ✓" : " ✗");
    
//     Serial.println("\nReed-Solomon Config:");
//     Serial.print("  Data: ");
//     Serial.print(DATA_LENGTH);
//     Serial.println(" bytes");
//     Serial.print("  ECC: ");
//     Serial.print(ECC_LENGTH);
//     Serial.println(" bytes");
//     Serial.print("  Total: ");
//     Serial.print(ENCODED_LENGTH);
//     Serial.println(" bytes");
//     Serial.print("  Correction: up to ");
//     Serial.print(ECC_LENGTH / 2);
//     Serial.println(" errors");
    
//     Serial.println("\nInitializing Bluetooth...");
//     bt.begin(115200);
//     bt.onDataReceived(onBluetoothData);
//     bt.setAutoEcho(false);
    
//     Serial.println("\n========================================");
//     Serial.println("COMMANDS (Serial or Bluetooth):");
//     Serial.println("  SEND        - Send with FEC (clean)");
//     Serial.println("  SEND_ERROR  - Send with simulated errors");
//     Serial.println("  SEND_PLAIN  - Send without FEC");
//     Serial.println("  TEST        - Test FEC locally");
//     Serial.println("  STATUS      - Show status");
//     Serial.println("========================================");
//     Serial.println("\n>> READY! Type 'TEST' first\n");
// }

// // ===== LOOP =====
// void loop() {
//     bt.update();
    
//     if (Serial.available()) {
//         String cmd = Serial.readStringUntil('\n');
//         cmd.trim();
        
//         if (cmd.equalsIgnoreCase("SEND")) {
//             sendWithFEC(false);
//         }
//         else if (cmd.equalsIgnoreCase("SEND_ERROR")) {
//             sendWithFEC(true);
//         }
//         else if (cmd.equalsIgnoreCase("SEND_PLAIN") || cmd.equalsIgnoreCase("PLAIN")) {
//             sendWithoutFEC();
//         }
//         else if (cmd.equalsIgnoreCase("TEST")) {
//             testFECLocal();
//         }
//         else if (cmd.equalsIgnoreCase("STATUS")) {
//             Serial.println("\n=== STATUS ===");
//             Serial.print("Connection: ");
//             Serial.println(bt.isConnected() ? "Connected" : "Disconnected");
//             Serial.print("Transfers: ");
//             Serial.println(sendCounter);
//             Serial.print("Data size: ");
//             Serial.print(DATA_LENGTH);
//             Serial.println(" bytes");
//             Serial.print("With ECC: ");
//             Serial.print(ENCODED_LENGTH);
//             Serial.println(" bytes");
//         }
//         else if (cmd.equalsIgnoreCase("HELP")) {
//             Serial.println("\nCOMMANDS:");
//             Serial.println("  SEND       - Send with FEC (no errors)");
//             Serial.println("  SEND_ERROR - Send with simulated errors");
//             Serial.println("  SEND_PLAIN - Send without FEC");
//             Serial.println("  TEST       - Test FEC locally");
//             Serial.println("  STATUS     - System status");
//         }
//     }
// }