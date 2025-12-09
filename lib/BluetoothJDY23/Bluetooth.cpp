/*
 * BluetoothJDY23.cpp
 * Library untuk komunikasi Bluetooth menggunakan module JDY-23
 * Author: Ahmad
 * Date: 2024
 */

#include "Bluetooth.h"

// Constructor
BluetoothJDY23::BluetoothJDY23(uint8_t rxPin, uint8_t txPin, 
                               uint8_t pwrPin, uint8_t statPin, 
                               uint8_t rstPin, uint8_t ledPin) {
    _rxPin = rxPin;
    _txPin = txPin;
    _pwrPin = pwrPin;
    _statPin = statPin;
    _rstPin = rstPin;
    _ledPin = ledPin;
    
    _connected = false;
    _autoEcho = true;
    _baudrate = 4800;
    _lastStatusCheck = 0;
    _dataCallback = nullptr;
    
    // Create hardware serial instance
    _serial = new HardwareSerial(_rxPin, _txPin);
}

// Initialize Bluetooth module
void BluetoothJDY23::begin(long baudrate) {
    _baudrate = baudrate;
    
    // Setup pins
    pinMode(_pwrPin, OUTPUT);
    pinMode(_rstPin, OUTPUT);
    pinMode(_ledPin, OUTPUT);
    pinMode(_statPin, INPUT);
    
    // Power on module
    digitalWrite(_pwrPin, HIGH);
    delay(50);
    
    // Reset module
    reset();
    
    // Initialize serial
    _serial->begin(_baudrate);
    
    // Initial LED blink
    for (int i = 0; i < 3; i++) {
        digitalWrite(_ledPin, HIGH);
        delay(100);
        digitalWrite(_ledPin, LOW);
        delay(100);
    }
    
    Serial.println("[BT] Bluetooth JDY-23 initialized");
    Serial.print("[BT] Baudrate: ");
    Serial.println(_baudrate);
}

// Update function - call this in loop()
void BluetoothJDY23::update() {
    // Check connection status every 500ms
    if (millis() - _lastStatusCheck >= 500) {
        checkConnection();
        _lastStatusCheck = millis();
    }
    
    updateLED();
    
    // Handle incoming data
    if (available()) {
        delay(20); // Wait for complete message at 4800 baud
        String data = readString();
        data.trim();
        
        if (data.length() > 0) {
            // Call callback if set
            if (_dataCallback != nullptr) {
                _dataCallback(data);
            }
            
            // Auto echo if enabled
            if (_autoEcho) {
                println("Received: " + data);
            }
        }
    }
}

// Send data to Bluetooth
void BluetoothJDY23::send(String data) {
    _serial->print(data);
}

void BluetoothJDY23::println(String data) {
    _serial->println(data);
}

// Check if data available
bool BluetoothJDY23::available() {
    return _serial->available();
}

// Read string from Bluetooth
String BluetoothJDY23::readString() {
    String data = "";
    while (_serial->available()) {
        char c = _serial->read();
        data += c;
    }
    return data;
}

// Read single character
char BluetoothJDY23::read() {
    return _serial->read();
}

// Reset module
void BluetoothJDY23::reset() {
    Serial.println("[BT] Resetting module...");
    digitalWrite(_rstPin, LOW);
    delay(20);
    digitalWrite(_rstPin, HIGH);
    delay(200);
    Serial.println("[BT] Reset complete");
}

// Check if connected
bool BluetoothJDY23::isConnected() {
    return _connected;
}

// Set callback for data received
void BluetoothJDY23::onDataReceived(void (*callback)(String data)) {
    _dataCallback = callback;
}

// Enable/disable auto echo
void BluetoothJDY23::setAutoEcho(bool enable) {
    _autoEcho = enable;
}

// Get current baudrate
long BluetoothJDY23::getBaudrate() {
    return _baudrate;
}

// Update LED status
void BluetoothJDY23::updateLED() {
    digitalWrite(_ledPin, _connected ? HIGH : LOW);
}

// Check connection status
void BluetoothJDY23::checkConnection() {
    bool currentStatus = digitalRead(_statPin);
    
    if (currentStatus != _connected) {
        _connected = currentStatus;
        
        if (_connected) {
            Serial.println("[BT] ✓ Connected!");
            // Blink LED twice when connected
            for (int i = 0; i < 2; i++) {
                digitalWrite(_ledPin, HIGH);
                delay(100);
                digitalWrite(_ledPin, LOW);
                delay(100);
            }
        } else {
            Serial.println("[BT] ✗ Disconnected");
        }
    }
}