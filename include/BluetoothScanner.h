/**
 * @file BluetoothScanner.h
 * @brief Bluetooth-Scanner für ESP32-C3
 * 
 * Diese Klasse implementiert das kontinuierliche Scannen nach
 * Bluetooth-Geräten und verwaltet die gefundenen Geräte.
 */

#ifndef BLUETOOTH_SCANNER_H
#define BLUETOOTH_SCANNER_H

#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <esp_task_wdt.h>
#include "Config.h"
#include "DeviceManager.h"

// Forward declaration
class BluetoothScanner;

/**
 * @brief Callback-Klasse für gefundene BLE-Geräte
 */
class SafeAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
private:
    BluetoothScanner* scanner;
    
public:
    SafeAdvertisedDeviceCallbacks(BluetoothScanner* btScanner) : scanner(btScanner) {}
    void onResult(BLEAdvertisedDevice advertisedDevice) override;
};

/**
 * @brief Bluetooth-Scanner Klasse
 */
class BluetoothScanner {
private:
    BLEScan* pBLEScan;
    DeviceManager* deviceManager;
    SafeAdvertisedDeviceCallbacks* callbacks;
    
    unsigned long lastScanTime;
    unsigned long lastSuccessfulScan;
    unsigned long lastBluetoothReset;
    unsigned long scanStartTime;        // Für automatische Scan-Zyklen
    bool scanCycleActive;               // True wenn im 2s+8s Zyklus
    int failedScansCount;
    bool currentlyScanning;
    bool initialized;
    
    // Statistics
    unsigned long sessionStartTime;
    int totalDevicesSeen;
    
public:
    BluetoothScanner();
    ~BluetoothScanner();
    
    // Initialization
    bool begin(DeviceManager* devMgr);
    void end();
    bool isInitialized() const { return initialized; }
    
    // Scanning
    void performScan();
    void performAutomaticScanCycle();   // Neuer automatischer Scan-Zyklus
    void resetBluetooth();
    bool isScanning() const { return currentlyScanning; }
    
    // Device processing (called by callback)
    void processDevice(BLEAdvertisedDevice& advertisedDevice);
    
    // Payload analysis
    String getManufacturerName(uint16_t companyId);
    String analyzeAppleDevice(uint8_t* data, size_t length);
    String analyzeSamsungDevice(uint8_t* data, size_t length);
    String analyzePayload(uint8_t* data, size_t length, uint16_t manufacturerId);
    
    // Statistics
    unsigned long getSessionStartTime() const { return sessionStartTime; }
    int getTotalDevicesSeen() const { return totalDevicesSeen; }
    unsigned long getLastSuccessfulScan() const { return lastSuccessfulScan; }
    int getFailedScansCount() const { return failedScansCount; }
};

#endif // BLUETOOTH_SCANNER_H