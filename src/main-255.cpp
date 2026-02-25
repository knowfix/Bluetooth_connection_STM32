// #include <Arduino.h>
// #include <RS-FEC.h>

// #define MSG_LEN     255
// #define ECC_LEN     32   // max corrected = 16 bytes

// // char message[] = "Ini adalah pesan uji coba sepanjang dua ratus lima puluh enam byte untuk "
// //                 "memastikan bahwa pengiriman data melalui modul Bluetooth berjalan stabil "
// //                 "tanpa kehilangan karakter atau korupsi data selama transmisi berlangsung. "
// //                 "Pengujian tambahan dilakukan.XXXXXXX";

// char message[] = "Ini adalah pesan uji coba Reed-Solomon FEC versi STRING "
//                  "dengan panjang maksimum 256 byte untuk pengujian "
//                  "Bluetooth UART STM32 dan Arduino.";

// char message_frame[MSG_LEN];
// char encoded[MSG_LEN + ECC_LEN];
// char repaired[MSG_LEN];

// RS::ReedSolomon<MSG_LEN, ECC_LEN> rs;

// void setup() {
//     Serial.begin(115200);
//     while (!Serial);

//     // Clear buffer
//     memset(message_frame, 0, sizeof(message_frame));

//     // Copy string ke frame (AMAN)
//     strncpy(message_frame, message, MSG_LEN - 1);

//     Serial.print("Original: ");
//     Serial.println(message_frame);

//     // ===== Encode =====
//     rs.Encode(message_frame, encoded);

//     Serial.print("Encoded:  ");
//     for (uint i = 0; i < sizeof(encoded); i++) {
//         Serial.print(encoded[i]);
//     }
//     Serial.println();

//     // ===== Simulasi corruption =====
//     for (uint i = 100; i < 115; i++) {
//         encoded[i] = '-';   // rusak 15 byte
//     }

//     Serial.print("Corrupted: ");
//     for (uint i = 0; i < sizeof(encoded); i++) {
//         Serial.print(encoded[i]);
//     }
//     Serial.println();

//     // ===== Decode =====
//     bool ok = rs.Decode(encoded, repaired);

//     Serial.print("Result: ");
//     Serial.println(ok ? "SUCCESS" : "FAILURE");

//     Serial.print("Repaired: ");
//     Serial.println(repaired);
// }

// void loop() {}
