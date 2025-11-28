/**
 * @file DeviceManager.h
 * @brief Geräte-Persistierung und Verwaltung
 * 
 * Diese Klasse verwaltet bekannte Geräte, lädt/speichert sie
 * aus dem Flash-Speicher und verwaltet die Geräteliste.
 */

#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include "Config.h"

// Device structure definition
struct SafeDevice {
    char address[18];  // "XX:XX:XX:XX:XX:XX"
    char name[32];
    char comment[50];  // Benutzer-Kommentar
    int rssi;
    unsigned long lastSeen;
    char lastSeenFormatted[20];  // "vor X min"
    bool isKnown;
    bool isActive;
    unsigned long firstSeenThisSession;
    int rssiThreshold;
    bool isNearby;
    bool wasNearbyLastTime;
    
    // Payload-Daten
    char manufacturer[32];
    char deviceType[32];
    char payloadHex[128];
    char serviceData[64];
    bool hasManufacturerData;
    bool hasServiceData;
    uint16_t manufacturerId;
};

#define MAX_DEVICES 32
#define MAX_KNOWN 200
#define MAX_OUTPUT_LOG_ENTRIES 30

// Output Log Entry für Ausgang-Schaltungen
struct OutputLogEntry {
    unsigned long timestamp;
    char deviceAddress[18];
    char deviceName[32];
    char comment[50];
    bool outputState;  // true = AN, false = AUS
    char reason[64];   // "Gerät erkannt", "Gerät verschwunden", etc.
};

/**
 * @brief Klasse für Geräte-Verwaltung
 */
class DeviceManager {
private:
    SafeDevice* devices;
    char knownMACs[MAX_KNOWN][18];
    char knownComments[MAX_KNOWN][MAX_COMMENT_LENGTH];
    int knownRSSIThresholds[MAX_KNOWN];
    int deviceCount;
    int knownCount;
    Preferences preferences;
    
    // Output Log System
    OutputLogEntry outputLog[MAX_OUTPUT_LOG_ENTRIES];
    int outputLogCount;
    int outputLogIndex;  // Ringpuffer-Index
    
    // Ever-seen tracking (Bitfeld für RAM-effizientes Tracking)
    uint32_t everSeenBitfield[8];  // 256 bits = 256 mögliche Geräte-Hashes
    int totalEverSeen;
    
public:
    DeviceManager();
    ~DeviceManager();
    
    // Initialization
    void begin(SafeDevice* deviceArray, int maxDevices);
    
    // Known devices management
    void loadKnownDevices();
    void saveKnownDevices();
    int addKnownDevice(const char* address, const char* comment, int rssiThreshold);
    bool removeKnownDevice(const char* address);
    bool isKnownDevice(const char* address);
    
    // Device management
    void updateDevice(const char* address, const char* name, int rssi);
    void updateManufacturerInfo(const char* address, const char* manufacturer, const char* deviceType, uint16_t manufacturerId, const char* payloadHex = "");
    void setDeviceActive(const char* address, bool active);
    void cleanupOldDevices();
    
    // Export/Import
    String exportDevicesJson();
    bool importDevicesJson(const String& jsonData);
    
    // Output Log System
    void logOutputChange(const char* deviceAddress, const char* deviceName, const char* comment, bool outputState, const char* reason);
    String getOutputLogJson();
    void clearOutputLog();
    
    // Getters
    int getDeviceCount() const { return deviceCount; }  // Aktiv (current seen)
    int getKnownCount() const { return knownCount; }    // Bekannt (saved)
    int getTotalEverSeen() const { return totalEverSeen; }  // Geräte ever seen
    int getActiveCount() const;  // Aktiv (current seen)
    int getPresentCount() const;  // Anwesend (current seen + near)
    SafeDevice* getDevices() { return devices; }
    char (*getKnownMACs())[18] { return knownMACs; }
    char (*getKnownComments())[MAX_COMMENT_LENGTH] { return knownComments; }
    int* getKnownRSSIThresholds() { return knownRSSIThresholds; }
};

#endif // DEVICE_MANAGER_H