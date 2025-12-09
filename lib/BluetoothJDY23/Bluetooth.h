/*
 * BluetoothJDY23.h
 * Library untuk komunikasi Bluetooth menggunakan module JDY-23
 * Author: Ahmad
 * Date: 2024
 */

#ifndef BLUETOOTH_JDY23_H
#define BLUETOOTH_JDY23_H

#include <Arduino.h>
#include <HardwareSerial.h>

class BluetoothJDY23 {
public:
    // Constructor
    BluetoothJDY23(uint8_t rxPin, uint8_t txPin, 
                   uint8_t pwrPin, uint8_t statPin, 
                   uint8_t rstPin, uint8_t ledPin);
    
    // Inisialisasi module
    void begin(long baudrate = 4800);
    
    // Update status LED dan koneksi
    void update();
    
    // Kirim data ke Bluetooth
    void send(String data);
    void println(String data);
    
    // Cek apakah ada data masuk
    bool available();
    
    // Baca data dari Bluetooth
    String readString();
    char read();
    
    // Reset module
    void reset();
    
    // Cek status koneksi
    bool isConnected();
    
    // Set callback untuk data received
    void onDataReceived(void (*callback)(String data));
    
    // Enable/disable auto echo
    void setAutoEcho(bool enable);
    
    // Get current baudrate
    long getBaudrate();

private:
    // Pin definitions
    uint8_t _rxPin, _txPin;
    uint8_t _pwrPin, _statPin, _rstPin, _ledPin;
    
    // Hardware serial
    HardwareSerial* _serial;
    
    // Status variables
    bool _connected;
    bool _autoEcho;
    long _baudrate;
    unsigned long _lastStatusCheck;
    
    // Callback function pointer
    void (*_dataCallback)(String data);
    
    // Internal functions
    void updateLED();
    void checkConnection();
};

#endif // BLUETOOTH_JDY23_H