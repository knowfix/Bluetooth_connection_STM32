/*
 * DataTransfer.cpp
 * Implementation untuk transfer data 256 bytes secara reliable
 * Author: Ahmad
 * Date: 2024
 */

#include "dataTransfer.h"

// Constructor
DataTransfer::DataTransfer(BluetoothJDY23* bluetooth) {
    _bt = bluetooth;
    _lastTransferTime = 0;
    _bytesSent = 0;
    _retryCount = 0;
    _debugMode = false;
}

// Send 256 bytes dengan chunking + ACK protocol
TransferStatus DataTransfer::send256Bytes(uint8_t* data) {
    return sendArray(data, 256);
}

// Send array dengan custom size
TransferStatus DataTransfer::sendArray(uint8_t* data, uint16_t size) {
    // Check connection
    if (!_bt->isConnected()) {
        Serial.println("[TRANSFER] Error: Not connected!");
        return TRANSFER_NO_CONNECTION;
    }
    
    // Reset statistics
    unsigned long startTime = millis();
    _bytesSent = 0;
    _retryCount = 0;
    
    // Calculate number of chunks
    uint16_t numChunks = (size + CHUNK_SIZE - 1) / CHUNK_SIZE;  // Ceiling division
    
    if (_debugMode) {
        Serial.print("[TRANSFER] Starting transfer: ");
        Serial.print(size);
        Serial.print(" bytes in ");
        Serial.print(numChunks);
        Serial.println(" chunks");
    }
    
    // Send start signal
    _bt->println("START_TRANSFER");
    delay(50);  // Give receiver time to prepare
    
    // Send each chunk
    for (uint16_t i = 0; i < numChunks; i++) {
        uint16_t offset = i * CHUNK_SIZE;
        uint8_t chunkSize = min(CHUNK_SIZE, size - offset);
        
        bool success = false;
        uint8_t retries = 0;
        
        // Retry mechanism
        while (!success && retries < MAX_RETRIES) {
            if (_debugMode) {
                Serial.print("[TRANSFER] Sending chunk ");
                Serial.print(i);
                Serial.print("/");
                Serial.print(numChunks - 1);
                if (retries > 0) {
                    Serial.print(" (retry ");
                    Serial.print(retries);
                    Serial.print(")");
                }
                Serial.println();
            }
            
            success = sendChunk(i, data + offset, chunkSize);
            
            if (!success) {
                retries++;
                _retryCount++;
                delay(50);  // Wait before retry
            }
        }
        
        // If failed after retries, abort transfer
        if (!success) {
            Serial.print("[TRANSFER] Failed at chunk ");
            Serial.println(i);
            _bt->println("TRANSFER_FAILED");
            return TRANSFER_FAILED;
        }
        
        _bytesSent += chunkSize;
    }
    
    // Send end signal
    _bt->println("END_TRANSFER");
    delay(20);
    
    // Calculate transfer time
    _lastTransferTime = millis() - startTime;
    
    if (_debugMode) {
        Serial.println("[TRANSFER] ✓ Transfer complete!");
        Serial.print("[TRANSFER] Bytes sent: ");
        Serial.println(_bytesSent);
        Serial.print("[TRANSFER] Time: ");
        Serial.print(_lastTransferTime);
        Serial.println(" ms");
        Serial.print("[TRANSFER] Speed: ");
        Serial.print((_bytesSent * 1000.0) / _lastTransferTime);
        Serial.println(" bytes/sec");
        Serial.print("[TRANSFER] Retries: ");
        Serial.println(_retryCount);
    }
    
    return TRANSFER_SUCCESS;
}

// Send single chunk with checksum
bool DataTransfer::sendChunk(uint8_t chunkIndex, uint8_t* data, uint8_t size) {
    // Calculate checksum
    uint8_t checksum = calculateChecksum(data, size);
    
    // Send chunk header: "CHUNK:<index>:<size>:<checksum>:"
    _bt->send("CHUNK:");
    _bt->send(String(chunkIndex));
    _bt->send(":");
    _bt->send(String(size));
    _bt->send(":");
    _bt->send(String(checksum));
    _bt->send(":");
    
    // Send data as hex string (reliable over Bluetooth)
    for (uint8_t i = 0; i < size; i++) {
        if (data[i] < 16) _bt->send("0");  // Padding for single digit hex
        _bt->send(String(data[i], HEX));
    }
    _bt->println("");  // End of chunk (newline)
    
    // Wait for ACK
    return waitForAck(chunkIndex);
}

// Wait for acknowledgment from receiver
bool DataTransfer::waitForAck(uint8_t expectedChunkIndex) {
    unsigned long startWait = millis();
    String response = "";
    
    while (millis() - startWait < ACK_TIMEOUT) {
        if (_bt->available()) {
            char c = _bt->read();
            
            if (c == '\n' || c == '\r') {
                // End of response
                response.trim();
                
                if (_debugMode) {
                    Serial.print("[TRANSFER] Received: ");
                    Serial.println(response);
                }
                
                // Check if ACK matches expected chunk
                if (response == "ACK:" + String(expectedChunkIndex)) {
                    return true;
                }
                
                // Reset for next line
                response = "";
            } else {
                response += c;
            }
        }
        delay(1);  // Small delay to not hog CPU
    }
    
    // Timeout
    if (_debugMode) {
        Serial.println("[TRANSFER] Timeout waiting for ACK");
    }
    return false;
}

// Simple checksum calculation
uint8_t DataTransfer::calculateChecksum(uint8_t* data, uint8_t size) {
    uint8_t checksum = 0;
    for (uint8_t i = 0; i < size; i++) {
        checksum ^= data[i];  // XOR checksum
    }
    return checksum;
}

// Get last transfer time
unsigned long DataTransfer::getLastTransferTime() {
    return _lastTransferTime;
}

// Get bytes sent in last transfer
uint16_t DataTransfer::getBytesSent() {
    return _bytesSent;
}

// Get retry count
uint8_t DataTransfer::getRetryCount() {
    return _retryCount;
}

// Enable/disable debug logging
void DataTransfer::setDebugMode(bool enable) {
    _debugMode = enable;
}