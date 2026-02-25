// #include <Arduino.h>

// /* =============================
//    Pin Konfigurasi Bluetooth JDY-23
//    ============================= */
// #define PIN_BT_PWR     PA8    // Power enable modul Bluetooth
// #define PIN_BT_STAT    PB10   // Status: HIGH = connected
// #define PIN_BT_RST     PC9    // Reset modul Bluetooth
// #define PIN_BT_LED     PC12   // LED indikator

// // HardwareSerial(RX_pin, TX_pin)
// HardwareSerial Serial1(PB7, PB6);

// /* =============================
//    Global Variables
//    ============================= */
// unsigned long lastPingTime = 0;
// const unsigned long PING_INTERVAL = 2000;
// bool lastBTStatus = false;
// int currentBaudIndex = 0;
// bool baudDetected = false;

// // Common baudrates to try (4800 first, as it's detected)
// const long baudRates[] = {4800, 9600, 19200, 38400, 57600, 115200, 14400, 28800};
// const int numBaudRates = sizeof(baudRates) / sizeof(baudRates[0]);

// /* =============================
//    Inisialisasi Bluetooth
//    ============================= */
// void initBluetooth() {
//   pinMode(PIN_BT_PWR, OUTPUT);
//   pinMode(PIN_BT_RST, OUTPUT);
//   pinMode(PIN_BT_LED, OUTPUT);
//   pinMode(PIN_BT_STAT, INPUT);

//   // Power ON modul
//   digitalWrite(PIN_BT_PWR, HIGH);
//   Serial.println("[INIT] Bluetooth power ON");
//   delay(50);

//   // Reset modul
//   digitalWrite(PIN_BT_RST, LOW);
//   delay(20);
//   digitalWrite(PIN_BT_RST, HIGH);
//   Serial.println("[INIT] Bluetooth reset done");
//   delay(200);

//   // Start with 4800 baud (detected baudrate for JDY-23)
//   Serial1.begin(4800);
//   Serial.print("[INIT] Starting with baudrate: ");
//   Serial.println(4800);
  
//   Serial.println("\n====================================");
//   Serial.println("  AUTO BAUDRATE DETECTION MODE");
//   Serial.println("  (Default: 4800 - already detected)");
//   Serial.println("====================================");
//   Serial.println("Instructions:");
//   Serial.println("1. Send message from your phone");
//   Serial.println("2. System will verify baudrate");
//   Serial.println("3. Or type NEXT to try other baudrates");
//   Serial.println("\nCurrent baudrate: " + String(baudRates[currentBaudIndex]));
//   Serial.println("====================================\n");
  
//   // Blink LED
//   for (int i = 0; i < 3; i++) {
//     digitalWrite(PIN_BT_LED, HIGH);
//     delay(100);
//     digitalWrite(PIN_BT_LED, LOW);
//     delay(100);
//   }
// }

// /* =============================
//    Check if received data is valid ASCII
//    ============================= */
// bool isValidASCII(String data) {
//   if (data.length() == 0) return false;
  
//   int validChars = 0;
//   for (int i = 0; i < data.length(); i++) {
//     char c = data[i];
//     // Check for printable ASCII (space to ~) or common control chars
//     if ((c >= 32 && c <= 126) || c == '\n' || c == '\r' || c == '\t') {
//       validChars++;
//     }
//   }
  
//   // If more than 70% are valid ASCII, consider it valid
//   return (validChars * 100 / data.length()) > 70;
// }

// /* =============================
//    Try next baudrate
//    ============================= */
// void tryNextBaudrate() {
//   currentBaudIndex++;
//   if (currentBaudIndex >= numBaudRates) {
//     currentBaudIndex = 0; // Loop back
//   }
  
//   Serial1.end();
//   delay(50);
//   Serial1.begin(baudRates[currentBaudIndex]);
  
//   Serial.println("\n----------------------------------------");
//   Serial.print("Trying baudrate: ");
//   Serial.println(baudRates[currentBaudIndex]);
//   Serial.println("Send 'TEST' from phone now...");
//   Serial.println("----------------------------------------\n");
// }

// /* =============================
//    LED Status
//    ============================= */
// void updateBTLed() {
//   bool connected = digitalRead(PIN_BT_STAT);
//   digitalWrite(PIN_BT_LED, connected);
  
//   if (connected != lastBTStatus) {
//     lastBTStatus = connected;
//     if (connected) {
//       Serial.println("\n[STATUS] ✓ Bluetooth CONNECTED!");
//     } else {
//       Serial.println("\n[STATUS] ✗ Bluetooth DISCONNECTED");
//     }
//   }
// }

// /* =============================
//    Setup
//    ============================= */
// void setup() {
//   Serial.begin(115200);
//   unsigned long t0 = millis();
//   while (!Serial && millis() - t0 < 3000) delay(10);

//   Serial.println("\n====================================");
//   Serial.println("  STM32F401 + JDY-23 Bluetooth");
//   Serial.println("  AUTO BAUDRATE DETECTOR");
//   Serial.println("====================================");

//   initBluetooth();
// }

// /* =============================
//    Loop
//    ============================= */
// void loop() {
//   updateBTLed();

//   // ===== Handle incoming data from Bluetooth =====
//   if (Serial1.available()) {
//     delay(20); // Wait for complete message at 4800 baud
//     String btMsg = "";
    
//     // Read all available data
//     while (Serial1.available()) {
//       char c = Serial1.read();
//       btMsg += c;
//     }
    
//     // Remove trailing newline/carriage return
//     btMsg.trim();
    
//     // Show raw data with hex for debugging
//     Serial.print("[BT->USB] ");
//     Serial.print(btMsg);
//     Serial.print(" (HEX: ");
//     for (int i = 0; i < btMsg.length() && i < 10; i++) {
//       Serial.print(btMsg[i], HEX);
//       Serial.print(" ");
//     }
//     Serial.println(")");
    
//     // Check if baudrate is correct
//     if (!baudDetected) {
//       if (isValidASCII(btMsg)) {
//         Serial.println("\n🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉");
//         Serial.println("✓✓✓ BAUDRATE DETECTED! ✓✓✓");
//         Serial.print("✓✓✓ Correct baudrate: ");
//         Serial.print(baudRates[currentBaudIndex]);
//         Serial.println(" ✓✓✓");
//         Serial.println("🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉\n");
        
//         baudDetected = true;
        
//         // Echo back
//         Serial1.println("STM32: Baudrate detected successfully!");
//       } else {
//         Serial.println("❌ Invalid data - trying next baudrate in 3 seconds...");
//         delay(3000);
//         tryNextBaudrate();
//       }
//     } else {
//       // Normal operation after baudrate detected
//       Serial1.print("STM32 received: ");
//       Serial1.println(btMsg);
//     }
//   }

//   // ===== Handle incoming data from USB Serial =====
//   if (Serial.available()) {
//     String usbMsg = "";
//     while (Serial.available()) {
//       char c = Serial.read();
//       usbMsg += c;
//       delay(2);
//     }
    
//     if (usbMsg.startsWith("NEXT")) {
//       tryNextBaudrate();
//     }
//     else if (usbMsg.startsWith("RST")) {
//       Serial.println("[CMD] Resetting Bluetooth...");
//       digitalWrite(PIN_BT_RST, LOW);
//       delay(20);
//       digitalWrite(PIN_BT_RST, HIGH);
//       delay(200);
//       Serial.println("[CMD] Reset done!");
//     }
//     else if (usbMsg.startsWith("STATUS")) {
//       Serial.print("[STATUS] Current baudrate: ");
//       Serial.println(baudRates[currentBaudIndex]);
//       Serial.print("[STATUS] Baudrate detected: ");
//       Serial.println(baudDetected ? "YES" : "NO");
//       Serial.print("[STATUS] Connected: ");
//       Serial.println(lastBTStatus ? "YES" : "NO");
//     }
//     else {
//       Serial.print("[USB->BT] ");
//       Serial.println(usbMsg);
//       Serial1.println(usbMsg);
//     }
//   }

//   // ===== Periodic ping (only after baudrate detected) =====
//   if (baudDetected && millis() - lastPingTime >= PING_INTERVAL) {
//     lastPingTime = millis();
//     Serial.println("[PING] Sending to Bluetooth...");
//     Serial1.println("Hello from STM32!");
//   }
// }