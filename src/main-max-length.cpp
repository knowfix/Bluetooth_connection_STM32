// #include <RS-FEC.h>
// #include <Arduino.h>
// // char message[] = "Some very important message that will be 256 bytes long..."; // Pesan Anda
// char message[] = "Ini adalah pesan uji coba sepanjang dua ratus lima puluh enam byte untuk "
//                 "memastikan bahwa pengiriman data melalui modul Bluetooth berjalan stabil "
//                 "tanpa kehilangan karakter atau korupsi data selama transmisi berlangsung. XXX";
// const int msglen = 223;        // Ubah dari 60 ke 256
// const uint8_t ECC_LENGTH = 32; // Tingkatkan ECC bytes (bisa koreksi hingga 16 byte error)

// char message_frame[msglen];
// char repaired[msglen];
// char encoded[msglen + ECC_LENGTH]; // Total: 288 bytes

// RS::ReedSolomon<msglen, ECC_LENGTH> rs;

// void setup() {
//   Serial.begin(115200);
//   while (!Serial);
 
//   memset(message_frame, 0, sizeof(message_frame));
  
//   // Salin pesan ke message_frame (perbaikan bug: gunakan i < msglen, bukan i <= msglen)
//   for(uint i = 0; i < msglen; i++) {
//     if(message[i] != '\0') {
//       message_frame[i] = message[i];
//     } else {
//       break; // Berhenti jika pesan sudah habis
//     }
//   }
 
//   rs.Encode(message_frame, encoded); 
 
//   Serial.print("Original: "); Serial.println(message_frame);
//   Serial.print("Encoded:  ");
//   for(uint i = 0; i < sizeof(encoded); i++) {
//     Serial.print(encoded[i]);
//   }
//   Serial.println("");
  
//   // Simulasi korupsi data (16 bytes rusak - masih bisa diperbaiki dengan ECC_LENGTH=32)
//   for(uint i = 50; i < 66; i++) {
//     encoded[i] = '-';
//   }
  
//   Serial.print("Corrupted: ");
//   for(uint i = 0; i < sizeof(encoded); i++) {
//     Serial.print(encoded[i]);
//   }
//   Serial.println("");
//   Serial.println(encoded);
//   rs.Decode(encoded, repaired);
  
//   String result;
//   memcmp(message_frame, repaired, msglen) == 0 ? result="SUCCESS" : result="FAILURE";
  
//   Serial.print("Result: ");
//   Serial.println(result);
//   Serial.print("Repaired: ");
//   Serial.println(repaired);
// }

// void loop() { }