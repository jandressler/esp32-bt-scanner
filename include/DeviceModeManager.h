/**
 * @file DeviceModeManager.h
 * @brief Verwaltung des Geräte-Betriebsmodus (Beacon vs. Scanner)
 * 
 * Diese Klasse verwaltet den Betriebsmodus des Geräts und ermöglicht
 * die Auswahl zwischen Beacon-Modus und Scanner-Modus über den Boot-Button.
 */

#ifndef DEVICE_MODE_MANAGER_H
#define DEVICE_MODE_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include "Config.h"

/**
 * @brief Beacon-Konfiguration
 */
struct BeaconConfig {
    char name[32];          // Beacon-Name (z.B. "BT-beacon_A1B2" oder custom)
    uint16_t intervalMs;    // Sendeintervall in Millisekunden
    int8_t txPower;         // TX Power Level (-12 bis +9 dBm)
    bool useCustomName;     // true = name verwenden, false = auto-generiert
};

/**
 * @brief Device Mode Manager Klasse
 */
class DeviceModeManager {
private:
    Preferences preferences;
    DeviceMode currentMode;
    BeaconConfig beaconConfig;
    bool isConfigured;
    unsigned long buttonPressStart;
    bool buttonPressed;
    
public:
    DeviceModeManager();
    ~DeviceModeManager();
    
    // Initialization
    bool begin();
    
    // Mode Management
    DeviceMode getCurrentMode() const { return currentMode; }
    bool isInBeaconMode() const { return currentMode == MODE_BEACON; }
    bool isInScannerMode() const { return currentMode == MODE_SCANNER; }
    bool isDeviceConfigured() const { return isConfigured; }
    
    // Mode Configuration
    bool setMode(DeviceMode mode);
    void saveMode();
    void loadMode();
    
    // Beacon Configuration
    BeaconConfig getBeaconConfig() const { return beaconConfig; }
    bool setBeaconConfig(const char* name, uint16_t intervalMs, int8_t txPower);
    void resetBeaconConfigToDefault();  // Reset to neutral mode
    void saveBeaconConfig();
    void loadBeaconConfig();
    String getBeaconName(); // Generiert Namen falls nicht custom
    
    // Button Handling
    void checkModeButton();
    bool isModeButtonPressed();
    
    // Reset
    void resetToDefaults();
    void clearAllSettings();
    
private:
    void setDefaults();
    String generateBeaconNameFromMAC();
};

#endif // DEVICE_MODE_MANAGER_H
