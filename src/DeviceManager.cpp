/**
 * @file DeviceManager.cpp
 * @brief Implementation der Geräte-Verwaltung
 */

#include "DeviceManager.h"

DeviceManager::DeviceManager() : devices(nullptr), deviceCount(0), knownCount(0), outputLogCount(0), outputLogIndex(0), totalEverSeen(0) {
    memset(knownMACs, 0, sizeof(knownMACs));
    memset(knownComments, 0, sizeof(knownComments));
    memset(knownRSSIThresholds, DEFAULT_RSSI_THRESHOLD, sizeof(knownRSSIThresholds));
    memset(outputLog, 0, sizeof(outputLog));
    memset(everSeenBitfield, 0, sizeof(everSeenBitfield));
}

DeviceManager::~DeviceManager() {
    // Nothing to clean up for now
}

void DeviceManager::begin(SafeDevice* deviceArray, int maxDevices) {
    devices = deviceArray;
    loadKnownDevices();
}

void DeviceManager::loadKnownDevices() {
    preferences.begin("known_devices", true);  // read-only
    
    knownCount = preferences.getInt("count", 0);
    if (knownCount > MAX_KNOWN) knownCount = MAX_KNOWN;
    
    for (int i = 0; i < knownCount; i++) {
        String macKey = "mac" + String(i);
        String commentKey = "comment" + String(i);
        String thresholdKey = "threshold" + String(i);
        
        String mac = preferences.getString(macKey.c_str(), "");
        String comment = preferences.getString(commentKey.c_str(), "");
        int threshold = preferences.getInt(thresholdKey.c_str(), DEFAULT_RSSI_THRESHOLD);
        
        if (mac.length() > 0) {
            strncpy(knownMACs[i], mac.c_str(), sizeof(knownMACs[i]) - 1);
            strncpy(knownComments[i], comment.c_str(), sizeof(knownComments[i]) - 1);
            knownRSSIThresholds[i] = threshold;
        }
    }
    
    preferences.end();
}

void DeviceManager::saveKnownDevices() {
    preferences.begin("known_devices", false);  // read-write
    
    preferences.putInt("count", knownCount);
    
    for (int i = 0; i < knownCount; i++) {
        String macKey = "mac" + String(i);
        String commentKey = "comment" + String(i);
        String thresholdKey = "threshold" + String(i);
        
        preferences.putString(macKey.c_str(), knownMACs[i]);
        preferences.putString(commentKey.c_str(), knownComments[i]);
        preferences.putInt(thresholdKey.c_str(), knownRSSIThresholds[i]);
    }
    
    preferences.end();
}

int DeviceManager::addKnownDevice(const char* address, const char* comment, int rssiThreshold) {
    if (knownCount >= MAX_KNOWN) {
        return -1;  // Array full
    }
    
    // Check if already known - if yes, update it
    for (int i = 0; i < knownCount; i++) {
        if (strcmp(knownMACs[i], address) == 0) {
            // Update existing entry
            strncpy(knownComments[i], comment, sizeof(knownComments[i]) - 1);
            knownRSSIThresholds[i] = rssiThreshold;
            saveKnownDevices();
            return i;  // Return existing index
        }
    }
    
    // Add new device
    strncpy(knownMACs[knownCount], address, sizeof(knownMACs[knownCount]) - 1);
    strncpy(knownComments[knownCount], comment, sizeof(knownComments[knownCount]) - 1);
    knownRSSIThresholds[knownCount] = rssiThreshold;
    knownCount++;
    
    saveKnownDevices();
    return knownCount - 1;
}

bool DeviceManager::removeKnownDevice(const char* address) {
    for (int i = 0; i < knownCount; i++) {
        if (strcmp(knownMACs[i], address) == 0) {
            // Shift remaining elements
            for (int j = i; j < knownCount - 1; j++) {
                strcpy(knownMACs[j], knownMACs[j + 1]);
                strcpy(knownComments[j], knownComments[j + 1]);
                knownRSSIThresholds[j] = knownRSSIThresholds[j + 1];
            }
            knownCount--;
            saveKnownDevices();
            return true;
        }
    }
    return false;
}

bool DeviceManager::isKnownDevice(const char* address) {
    for (int i = 0; i < knownCount; i++) {
        if (strcmp(knownMACs[i], address) == 0) {
            return true;
        }
    }
    return false;
}

void DeviceManager::updateDevice(const char* address, const char* name, int rssi) {
    // Ever-seen tracking: Simple Hash über MAC-Adresse
    uint32_t hash = 0;
    for (int i = 0; address[i] != '\0'; i++) {
        hash = hash * 31 + (uint8_t)address[i];
    }
    uint8_t bitIndex = hash % 256;
    uint8_t arrayIndex = bitIndex / 32;
    uint8_t bitPosition = bitIndex % 32;
    
    // Prüfe ob Gerät bereits jemals gesehen wurde
    if (!(everSeenBitfield[arrayIndex] & (1 << bitPosition))) {
        everSeenBitfield[arrayIndex] |= (1 << bitPosition);
        totalEverSeen++;
    }
    
    // Find existing device or create new one
    int deviceIndex = -1;
    for (int i = 0; i < deviceCount; i++) {
        if (strcmp(devices[i].address, address) == 0) {
            deviceIndex = i;
            break;
        }
    }
    
    if (deviceIndex == -1) {
        if (deviceCount < MAX_DEVICES) {
            // Create new device
            deviceIndex = deviceCount++;
        } else {
            // LRU-Ersatz: Ältestes Gerät (kleinstes lastSeen) ersetzen
            unsigned long oldestTs = ULONG_MAX;
            int oldestIdx = 0;
            for (int i = 0; i < deviceCount; i++) {
                unsigned long ts = devices[i].lastSeen;
                if (ts == 0 || ts < oldestTs) {
                    oldestTs = ts;
                    oldestIdx = i;
                }
            }
            deviceIndex = oldestIdx;
        }
        memset(&devices[deviceIndex], 0, sizeof(SafeDevice));
        strncpy(devices[deviceIndex].address, address, sizeof(devices[deviceIndex].address) - 1);
        devices[deviceIndex].firstSeenThisSession = millis();
        devices[deviceIndex].rssiThreshold = DEFAULT_RSSI_THRESHOLD;
        strcpy(devices[deviceIndex].manufacturer, "Unbekannt"); // Initialize as unknown
    }
    
    if (deviceIndex >= 0) {
        // Update device - nur bessere/neue Informationen übernehmen
        
        // Name nur aktualisieren wenn der neue Name besser ist
        if (name && strlen(name) > 0) {
            // Neuer Name ist vorhanden
            if (strlen(devices[deviceIndex].name) == 0 || 
                strcmp(devices[deviceIndex].name, "Unbekanntes Gerät") == 0) {
                // Alter Name war leer oder "Unbekanntes Gerät" -> neuen Namen übernehmen
                strncpy(devices[deviceIndex].name, name, sizeof(devices[deviceIndex].name) - 1);
                devices[deviceIndex].name[sizeof(devices[deviceIndex].name) - 1] = '\0';
            }
        }
        
        devices[deviceIndex].rssi = rssi;
        devices[deviceIndex].lastSeen = millis();
        devices[deviceIndex].isActive = true;
        devices[deviceIndex].isKnown = isKnownDevice(address);
        
        // Set comment and threshold for known devices
        if (devices[deviceIndex].isKnown) {
            for (int i = 0; i < knownCount; i++) {
                if (strcmp(knownMACs[i], address) == 0) {
                    strncpy(devices[deviceIndex].comment, knownComments[i], sizeof(devices[deviceIndex].comment) - 1);
                    devices[deviceIndex].rssiThreshold = knownRSSIThresholds[i];
                    break;
                }
            }
        }
        
        // Update isNearby status
        devices[deviceIndex].wasNearbyLastTime = devices[deviceIndex].isNearby;
        devices[deviceIndex].isNearby = (rssi >= devices[deviceIndex].rssiThreshold);
        
        // Format lastSeen time
        unsigned long timeDiff = (millis() - devices[deviceIndex].lastSeen) / 1000;
        if (timeDiff < 60) {
            snprintf(devices[deviceIndex].lastSeenFormatted, sizeof(devices[deviceIndex].lastSeenFormatted), "vor %lus", timeDiff);
        } else {
            snprintf(devices[deviceIndex].lastSeenFormatted, sizeof(devices[deviceIndex].lastSeenFormatted), "vor %lum", timeDiff / 60);
        }
    }
}

void DeviceManager::updateManufacturerInfo(const char* address, const char* manufacturer, const char* deviceType, uint16_t manufacturerId, const char* payloadHex) {
    for (int i = 0; i < deviceCount; i++) {
        if (strcmp(devices[i].address, address) == 0) {
            // Hersteller nur aktualisieren wenn besser/neuer
            if (manufacturer && strlen(manufacturer) > 0) {
                if (strlen(devices[i].manufacturer) == 0 || 
                    strcmp(devices[i].manufacturer, "Unbekannt") == 0) {
                    strncpy(devices[i].manufacturer, manufacturer, sizeof(devices[i].manufacturer) - 1);
                    devices[i].manufacturer[sizeof(devices[i].manufacturer) - 1] = '\0';
                }
            }
            
            // Device Type erweitern
            if (deviceType && strlen(deviceType) > 0 && strlen(devices[i].deviceType) == 0) {
                strncpy(devices[i].deviceType, deviceType, sizeof(devices[i].deviceType) - 1);
                devices[i].deviceType[sizeof(devices[i].deviceType) - 1] = '\0';
            }
            
            // Payload-Daten erweitern/aktualisieren (immer neueste nehmen)
            if (payloadHex && strlen(payloadHex) > 0) {
                strncpy(devices[i].payloadHex, payloadHex, sizeof(devices[i].payloadHex) - 1);
                devices[i].payloadHex[sizeof(devices[i].payloadHex) - 1] = '\0';
            }
            
            // Manufacturer ID aktualisieren wenn wir neue Daten haben
            if (manufacturerId != 0) {
                devices[i].manufacturerId = manufacturerId;
            }
            
            devices[i].hasManufacturerData = true;
            break;
        }
    }
}

void DeviceManager::setDeviceActive(const char* address, bool active) {
    for (int i = 0; i < deviceCount; i++) {
        if (strcmp(devices[i].address, address) == 0) {
            devices[i].isActive = active;
            break;
        }
    }
}

void DeviceManager::cleanupOldDevices() {
    unsigned long currentTime = millis();
    
    // Rückwärts durch die Liste gehen um beim Löschen keine Indizes zu überspringen
    for (int i = deviceCount - 1; i >= 0; i--) {
        if (devices[i].isActive && (currentTime - devices[i].lastSeen) > DEVICE_TIMEOUT_MS) {
            // Gerät komplett entfernen (falls nicht bekannt)
            if (!devices[i].isKnown) {
                // Alle nachfolgenden Geräte nach vorne verschieben
                for (int j = i; j < deviceCount - 1; j++) {
                    devices[j] = devices[j + 1];
                }
                deviceCount--;
            } else {
                // Bekannte Geräte nur als inaktiv markieren
                devices[i].isActive = false;
            }
        }
    }
}

String DeviceManager::exportDevicesJson() {
    JsonDocument doc;
    JsonArray knownArray = doc["knownDevices"].to<JsonArray>();
    
    for (int i = 0; i < knownCount; i++) {
        JsonObject knownObj = knownArray.add<JsonObject>();
        knownObj["address"] = knownMACs[i];
        knownObj["comment"] = knownComments[i];
        knownObj["rssiThreshold"] = knownRSSIThresholds[i];
    }
    
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

bool DeviceManager::importDevicesJson(const String& jsonData) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonData);
    
    if (error) {
        logOutputChange("", "", "", false, "Import fehlgeschlagen: JSON ungültig");
        return false;
    }
    
    if (!doc["knownDevices"].is<JsonArray>()) {
        logOutputChange("", "", "", false, "Import fehlgeschlagen: knownDevices fehlt");
        return false;
    }
    
    JsonArray knownArray = doc["knownDevices"];
    int importCount = 0;
    int updateCount = 0;
    int skippedCount = 0;
    int startCount = knownCount;
    
    for (JsonObject knownObj : knownArray) {
        const char* address = knownObj["address"];
        const char* comment = knownObj["comment"] | "";
        int rssiThreshold = knownObj["rssiThreshold"] | DEFAULT_RSSI_THRESHOLD;
        
        if (address && strlen(address) > 0) {
            // Check if device existed before
            bool existed = false;
            for (int i = 0; i < startCount; i++) {
                if (strcmp(knownMACs[i], address) == 0) {
                    existed = true;
                    break;
                }
            }
            
            int result = addKnownDevice(address, comment, rssiThreshold);
            if (result >= 0) {
                if (existed) {
                    updateCount++;
                } else {
                    importCount++;
                }
            } else {
                skippedCount++;
            }
        }
    }
    
    // Einzige Log-Meldung mit Zusammenfassung
    char logMsg[120];
    if (importCount > 0 || updateCount > 0) {
        snprintf(logMsg, sizeof(logMsg), "📥 Import: %d neu, %d aktualisiert%s", 
                 importCount, updateCount, skippedCount > 0 ? ", einige übersprungen" : "");
    } else {
        snprintf(logMsg, sizeof(logMsg), "📥 Import: Keine Änderungen (%d bereits vorhanden)", updateCount + importCount);
    }
    logOutputChange("", "", "", false, logMsg);
    
    return (importCount + updateCount + skippedCount) > 0;
}

// Output Log System Implementation
void DeviceManager::logOutputChange(const char* deviceAddress, const char* deviceName, const char* comment, bool outputState, const char* reason) {
    // Ringpuffer-Index berechnen
    outputLogIndex = outputLogCount % MAX_OUTPUT_LOG_ENTRIES;
    
    // Neuen Eintrag erstellen
    OutputLogEntry& entry = outputLog[outputLogIndex];
    entry.timestamp = millis();
    strncpy(entry.deviceAddress, deviceAddress, sizeof(entry.deviceAddress) - 1);
    strncpy(entry.deviceName, deviceName, sizeof(entry.deviceName) - 1);
    strncpy(entry.comment, comment, sizeof(entry.comment) - 1);
    entry.outputState = outputState;
    strncpy(entry.reason, reason, sizeof(entry.reason) - 1);
    
    // Strings null-terminieren
    entry.deviceAddress[sizeof(entry.deviceAddress) - 1] = '\0';
    entry.deviceName[sizeof(entry.deviceName) - 1] = '\0';
    entry.comment[sizeof(entry.comment) - 1] = '\0';
    entry.reason[sizeof(entry.reason) - 1] = '\0';
    
    outputLogCount++;
}

String DeviceManager::getOutputLogJson() {
    JsonDocument doc;
    JsonArray logArray = doc["outputLog"].to<JsonArray>();
    
    // Anzahl der Einträge (maximal MAX_OUTPUT_LOG_ENTRIES)
    int entriesToShow = min(outputLogCount, MAX_OUTPUT_LOG_ENTRIES);
    
    // Vom neuesten zum ältesten (rückwärts durch Ringpuffer)
    for (int i = 0; i < entriesToShow; i++) {
        int index = (outputLogIndex - i + MAX_OUTPUT_LOG_ENTRIES) % MAX_OUTPUT_LOG_ENTRIES;
        const OutputLogEntry& entry = outputLog[index];
        
        JsonObject logEntry = logArray.add<JsonObject>();
        logEntry["timestamp"] = entry.timestamp;
        logEntry["deviceAddress"] = entry.deviceAddress;
        logEntry["deviceName"] = entry.deviceName;
        logEntry["comment"] = entry.comment;
        logEntry["outputState"] = entry.outputState;
        logEntry["reason"] = entry.reason;
        
        // Zeitstempel formatieren (relativ)
        unsigned long elapsedMs = millis() - entry.timestamp;
        if (elapsedMs < 60000) {
            logEntry["timeAgo"] = String(elapsedMs / 1000) + "s";
        } else if (elapsedMs < 3600000) {
            logEntry["timeAgo"] = String(elapsedMs / 60000) + "m";
        } else {
            logEntry["timeAgo"] = String(elapsedMs / 3600000) + "h";
        }
    }
    
    doc["totalEntries"] = outputLogCount;
    doc["maxEntries"] = MAX_OUTPUT_LOG_ENTRIES;
    
    String result;
    serializeJson(doc, result);
    return result;
}

void DeviceManager::clearOutputLog() {
    outputLogCount = 0;
    outputLogIndex = 0;
    memset(outputLog, 0, sizeof(outputLog));
}

int DeviceManager::getActiveCount() const {
    int count = 0;
    for (int i = 0; i < deviceCount; i++) {
        if (devices[i].isActive) {
            count++;
        }
    }
    return count;
}

int DeviceManager::getPresentCount() const {
    int count = 0;
    for (int i = 0; i < deviceCount; i++) {
        if (devices[i].isActive && devices[i].isKnown && devices[i].rssi >= devices[i].rssiThreshold) {
            count++;
        }
    }
    return count;
}