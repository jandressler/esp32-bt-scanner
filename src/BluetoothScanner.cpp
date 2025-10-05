/**
 * @file BluetoothScanner.cpp
 * @brief Implementierung der Bluetooth-Scanner Klasse
 */

#include "BluetoothScanner.h"

// =================== SafeAdvertisedDeviceCallbacks Implementation ===================

void SafeAdvertisedDeviceCallbacks::onResult(BLEAdvertisedDevice advertisedDevice) {
    if (scanner) {
        scanner->processDevice(advertisedDevice);
    }
}

// =================== BluetoothScanner Implementation ===================

BluetoothScanner::BluetoothScanner() 
    : pBLEScan(nullptr), deviceManager(nullptr), callbacks(nullptr),
      lastScanTime(0), lastSuccessfulScan(0), lastBluetoothReset(0),
      scanStartTime(0), scanCycleActive(false),
      failedScansCount(0), currentlyScanning(false), initialized(false),
      sessionStartTime(0), totalDevicesSeen(0) {
}

BluetoothScanner::~BluetoothScanner() {
    end();
}

bool BluetoothScanner::begin(DeviceManager* devMgr) {
    BT_DEBUG_PRINTF("BT-Scan: Initialisierung gestartet\n");
    
    if (!devMgr) {
        BT_DEBUG_PRINTF("BT-Scan: FEHLER - Kein DeviceManager übergeben\n");
        return false;
    }
    
    deviceManager = devMgr;
    
    // BLE initialisieren
    if (!BLEDevice::getInitialized()) {
        BLEDevice::init("ESP32-BT-Scanner");
        delay(100);
    }
    
    // Scanner erstellen
    pBLEScan = BLEDevice::getScan();
    if (!pBLEScan) {
        BT_DEBUG_PRINTF("BT-Scan: FEHLER - Kann BLE-Scanner nicht erstellen\n");
        return false;
    }
    
    // Callbacks erstellen
    callbacks = new SafeAdvertisedDeviceCallbacks(this);
    pBLEScan->setAdvertisedDeviceCallbacks(callbacks);
    
    // Scanner konfigurieren
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
    
    sessionStartTime = millis();
    initialized = true;
    
    BT_DEBUG_PRINTF("BT-Scan: Initialisierung erfolgreich\n");
    return true;
}

void BluetoothScanner::end() {
    if (!initialized) return;
    
    if (currentlyScanning) {
        pBLEScan->stop();
        currentlyScanning = false;
    }
    
    if (pBLEScan) {
        pBLEScan->clearResults();
    }
    
    if (callbacks) {
        delete callbacks;
        callbacks = nullptr;
    }
    
    initialized = false;
    BT_DEBUG_PRINTF("BT-Scan: Beendet\n");
}

void BluetoothScanner::performScan() {
    if (!initialized || currentlyScanning) {
        return;
    }
    
    unsigned long now = millis();
    
    // Rate limiting
    if (now - lastScanTime < BT_SCAN_INTERVAL_MS) {
        return;
    }
    
    lastScanTime = now;
    
    BT_DEBUG_PRINTF("BT-Scan: Starte manuellen Scan...\n");
    
    try {
        currentlyScanning = true;
        
        // Alten Scan-Cache leeren
        pBLEScan->clearResults();
        
        // Scan starten (non-blocking)
        pBLEScan->start(BT_SCAN_DURATION_SEC, false);
        
        lastSuccessfulScan = now;
        failedScansCount = 0;
        
        // Scan-Ergebnisse verarbeiten
        BLEScanResults foundDevices = pBLEScan->getResults();
        
        BT_DEBUG_PRINTF("BT-Scan: %d Geräte gefunden\n", foundDevices.getCount());
        
        currentlyScanning = false;
        
    } catch (const std::exception& e) {
        currentlyScanning = false;
        failedScansCount++;
        
        BT_DEBUG_PRINTF("BT-Scan: FEHLER - %s\n", e.what());
        
        // Nach mehreren Fehlern Bluetooth reset
        if (failedScansCount >= BT_MAX_FAILED_SCANS) {
            resetBluetooth();
        }
    }
}

void BluetoothScanner::performAutomaticScanCycle() {
    if (!initialized) {
        return;
    }
    
    unsigned long now = millis();
    
    if (!scanCycleActive) {
        // Starte neuen Scan-Zyklus
        scanCycleActive = true;
        scanStartTime = now;
        
        // Alte Geräte bereinigen vor neuem Scan
        deviceManager->cleanupOldDevices();
        
        try {
            currentlyScanning = true;
            
            // Alten Scan-Cache leeren
            pBLEScan->clearResults();
            
            // Scan starten (non-blocking)
            pBLEScan->start(BT_SCAN_DURATION_SEC, false);
            
            BT_DEBUG_PRINTF("BT-Auto-Scan: Gestartet (2s)\n");
            
        } catch (const std::exception& e) {
            currentlyScanning = false;
            scanCycleActive = false;
            failedScansCount++;
            
            BT_DEBUG_PRINTF("BT-Auto-Scan: FEHLER - %s\n", e.what());
            
            if (failedScansCount >= BT_MAX_FAILED_SCANS) {
                resetBluetooth();
            }
        }
    } else {
        // Prüfe Scan-Zyklus Status
        unsigned long elapsed = now - scanStartTime;
        
        if (currentlyScanning && elapsed >= (BT_SCAN_DURATION_SEC * 1000)) {
            // Scan-Phase beendet
            currentlyScanning = false;
            
            // Scan-Ergebnisse verarbeiten
            BLEScanResults foundDevices = pBLEScan->getResults();
            BT_DEBUG_PRINTF("BT-Auto-Scan: %d Geräte gefunden, starte 8s Pause\n", foundDevices.getCount());
            
            lastSuccessfulScan = now;
            failedScansCount = 0;
        }
        
        if (elapsed >= BT_SCAN_INTERVAL_MS) {
            // Gesamtzyklus beendet (2s scan + 8s pause)
            scanCycleActive = false;
            BT_DEBUG_PRINTF("BT-Auto-Scan: Zyklus beendet\n");
        }
    }
}

void BluetoothScanner::resetBluetooth() {
    BT_DEBUG_PRINTF("BT-Scan: Bluetooth-Reset wird durchgeführt...\n");
    
    end();
    delay(1000);
    
    // BLE komplett neu initialisieren
    esp_bt_controller_disable();
    delay(500);
    
    lastBluetoothReset = millis();
    failedScansCount = 0;
    
    // Neu initialisieren
    begin(deviceManager);
    
    BT_DEBUG_PRINTF("BT-Scan: Bluetooth-Reset abgeschlossen\n");
}

void BluetoothScanner::processDevice(BLEAdvertisedDevice& advertisedDevice) {
    String address = advertisedDevice.getAddress().toString().c_str();
    String name = "";
    
    // Gerätename falls verfügbar
    if (advertisedDevice.haveName()) {
        name = advertisedDevice.getName().c_str();
    }
    
    int rssi = advertisedDevice.getRSSI();
    
    // Prüfen ob Gerät bereits bekannt ist
    bool deviceFound = false;
    
    if (!deviceFound) {
        // Neues Gerät erstellen
        SafeDevice newDevice = {};  // Struktur initialisieren
        strncpy(newDevice.address, address.c_str(), sizeof(newDevice.address) - 1);
        strncpy(newDevice.name, name.c_str(), sizeof(newDevice.name) - 1);
        newDevice.rssi = rssi;
        newDevice.lastSeen = millis();
        newDevice.firstSeenThisSession = millis();
        newDevice.isKnown = false;
        newDevice.isActive = true;
        newDevice.isNearby = true;
        newDevice.hasManufacturerData = false;
        newDevice.hasServiceData = false;
        
        // Geräteerkennung basierend auf Namen
        if (name.indexOf("iPhone") >= 0 || name.indexOf("iPad") >= 0) {
            strncpy(newDevice.deviceType, "Apple Device", sizeof(newDevice.deviceType) - 1);
        } else if (name.indexOf("Galaxy") >= 0 || name.indexOf("Samsung") >= 0) {
            strncpy(newDevice.deviceType, "Samsung Device", sizeof(newDevice.deviceType) - 1);
        } else if (name.indexOf("Pixel") >= 0) {
            strncpy(newDevice.deviceType, "Google Device", sizeof(newDevice.deviceType) - 1);
        } else if (advertisedDevice.haveServiceUUID()) {
            strncpy(newDevice.deviceType, "BLE Service Device", sizeof(newDevice.deviceType) - 1);
        } else {
            strncpy(newDevice.deviceType, "Unknown", sizeof(newDevice.deviceType) - 1);
        }
        
        // Manufacturer Data analysieren
        String allPayloadData = "";
        
        if (advertisedDevice.haveManufacturerData()) {
            std::string manufData = advertisedDevice.getManufacturerData();
            uint8_t* data = (uint8_t*)manufData.data();
            size_t length = manufData.length();
            
            newDevice.hasManufacturerData = true;
            
            if (length >= 2) {
                uint16_t manufacturerId = (data[1] << 8) | data[0];
                newDevice.manufacturerId = manufacturerId;
                
                // Rohe Hex-Daten erstellen
                String hexData = "";
                for (size_t i = 0; i < length && i < 32; i++) {
                    if (i > 0) hexData += " ";
                    if (data[i] < 0x10) hexData += "0";
                    hexData += String(data[i], HEX);
                }
                allPayloadData = hexData;
                
                String manufName = getManufacturerName(manufacturerId);
                strncpy(newDevice.manufacturer, manufName.c_str(), sizeof(newDevice.manufacturer) - 1);
            }
        }
        
        // Service Data sammeln wenn keine Manufacturer Data vorhanden
        if (allPayloadData.length() == 0 && advertisedDevice.haveServiceData()) {
            newDevice.hasServiceData = true;
            allPayloadData = "ServiceData:YES";
        }
        
        // Falls keine spezifischen Daten verfügbar, sammle andere Advertising-Infos
        if (allPayloadData.length() == 0) {
            String advInfo = "";
            if (advertisedDevice.haveName()) {
                advInfo += "N:" + name + " ";
            }
            if (advertisedDevice.haveTXPower()) {
                advInfo += "TX:" + String(advertisedDevice.getTXPower()) + "dBm ";
            }
            if (advertisedDevice.haveAppearance()) {
                advInfo += "App:0x" + String(advertisedDevice.getAppearance(), HEX) + " ";
            }
            if (advertisedDevice.haveServiceUUID()) {
                advInfo += "SVC:YES ";
            }
            
            allPayloadData = advInfo.length() > 0 ? advInfo : "AdvData:NONE";
        }
        
        // Payload-Daten immer speichern
        strncpy(newDevice.payloadHex, allPayloadData.c_str(), sizeof(newDevice.payloadHex) - 1);
        
        // Gerät zum DeviceManager hinzufügen/aktualisieren
        deviceManager->updateDevice(address.c_str(), name.c_str(), rssi);
        
        // Hersteller-Informationen und Payload-Daten immer aktualisieren
        deviceManager->updateManufacturerInfo(address.c_str(), newDevice.manufacturer, newDevice.deviceType, newDevice.manufacturerId, newDevice.payloadHex);
        
        totalDevicesSeen++;
        
        BT_DEBUG_PRINTF("BT-Scan: Gefunden - %s (%s) RSSI: %d\n", 
                       name.c_str(), address.c_str(), rssi);
    }
}

String BluetoothScanner::getManufacturerName(uint16_t companyId) {
    switch (companyId) {
        case 0x004C: return "Apple";
        case 0x0075: return "Samsung";
        case 0x00E0: return "Google";
        case 0x0006: return "Microsoft";
        case 0x000F: return "Broadcom";
        case 0x0087: return "Garmin";
        case 0x0131: return "Cypress Semiconductor";
        default: return "Unknown (" + String(companyId, HEX) + ")";
    }
}

String BluetoothScanner::analyzeAppleDevice(uint8_t* data, size_t length) {
    if (length < 3) return "Apple (insufficient data)";
    
    uint8_t type = data[2];
    switch (type) {
        case 0x02: return "Apple iBeacon";
        case 0x05: return "Apple AirDrop";
        case 0x07: return "Apple AirPods";
        case 0x09: return "Apple Watch";
        case 0x0A: return "Apple Handoff";
        case 0x0C: return "Apple Action Tag";
        case 0x10: return "Apple Nearby";
        default: return "Apple Device (Type: " + String(type, HEX) + ")";
    }
}

String BluetoothScanner::analyzeSamsungDevice(uint8_t* data, size_t length) {
    if (length < 3) return "Samsung (insufficient data)";
    
    // Samsung-spezifische Analyse
    return "Samsung Device";
}

String BluetoothScanner::analyzePayload(uint8_t* data, size_t length, uint16_t manufacturerId) {
    if (!data || length < 2) return "No data";
    
    switch (manufacturerId) {
        case 0x004C: // Apple
            return analyzeAppleDevice(data, length);
        case 0x0075: // Samsung
            return analyzeSamsungDevice(data, length);
        default:
            // Return raw hex data for unknown manufacturers
            String hexStr = "";
            for (size_t i = 0; i < length && i < 32; i++) {  // Limit to 32 bytes to avoid overflow
                if (i > 0) hexStr += " ";
                if (data[i] < 0x10) hexStr += "0";
                hexStr += String(data[i], HEX);
            }
            return hexStr.length() > 0 ? hexStr : "No data";
    }
}