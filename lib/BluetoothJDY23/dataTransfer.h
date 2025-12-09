/*
 * DataTransfer.h
 * Extension untuk transfer data 256 bytes secara reliable
 * Author: Ahmad
 * Date: 2024
 */

#ifndef DATA_TRANSFER_H
#define DATA_TRANSFER_H

#include <Arduino.h>
#include <Bluetooth.h>

// Transfer configuration
#define CHUNK_SIZE 16           // Kirim 16 bytes per chunk
#define ACK_TIMEOUT 100         // Timeout menunggu ACK (ms)
#define MAX_RETRIES 3           // Maksimal retry per chunk

// Transfer status codes
enum TransferStatus {
    TRANSFER_SUCCESS = 0,
    TRANSFER_TIMEOUT = 1,
    TRANSFER_FAILED = 2,
    TRANSFER_NO_CONNECTION = 3
};

class DataTransfer {
public:
    // Constructor
    DataTransfer(BluetoothJDY23* bluetooth);
    
    // Send 256 bytes dengan chunking + ACK
    TransferStatus send256Bytes(uint8_t* data);
    
    // Send array dengan custom size
    TransferStatus sendArray(uint8_t* data, uint16_t size);
    
    // Get transfer statistics
    unsigned long getLastTransferTime();
    uint16_t getBytesSent();
    uint8_t getRetryCount();
    
    // Enable/disable detailed logging
    void setDebugMode(bool enable);

private:
    BluetoothJDY23* _bt;
    
    // Statistics
    unsigned long _lastTransferTime;
    uint16_t _bytesSent;
    uint8_t _retryCount;
    bool _debugMode;
    
    // Internal functions
    bool sendChunk(uint8_t chunkIndex, uint8_t* data, uint8_t size);
    bool waitForAck(uint8_t expectedChunkIndex);
    uint8_t calculateChecksum(uint8_t* data, uint8_t size);
};

#endif // DATA_TRANSFER_H