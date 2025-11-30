/**
 * @file DeviceModeManager.cpp
 * @brief Implementation der Geräte-Modus-Verwaltung
 */

#include "DeviceModeManager.h"
#include <BLEDevice.h>

DeviceModeManager::DeviceModeManager() 
    : currentMode(MODE_BEACON), isConfigured(false), buttonPressStart(0), buttonPressed(false) {
    setDefaults();
}

DeviceModeManager::~DeviceModeManager() {
    preferences.end();
}

bool DeviceModeManager::begin() {
    // Configure mode button
    pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
    
    // Load saved configuration
    loadMode();
    loadBeaconConfig();
    
    // Check if device was ever configured
    preferences.begin("mode", true);
    isConfigured = preferences.getBool("configured", false);
    preferences.end();
    
    // First boot: Default to Beacon mode
    if (!isConfigured) {
        currentMode = MODE_BEACON;
        isConfigured = true;
        preferences.begin("mode", false);
        preferences.putBool("configured", true);
        preferences.end();
    }
    
    return true;
}

void DeviceModeManager::setDefaults() {
    // Beacon defaults
    beaconConfig.intervalMs = BEACON_DEFAULT_INTERVAL_MS;
    beaconConfig.txPower = BEACON_DEFAULT_TX_POWER;
    beaconConfig.useCustomName = false;
    strncpy(beaconConfig.name, BEACON_DEFAULT_NAME, sizeof(beaconConfig.name) - 1);
    beaconConfig.name[sizeof(beaconConfig.name) - 1] = '\0';
}

bool DeviceModeManager::setMode(DeviceMode mode) {
    currentMode = mode;
    saveMode();
    return true;
}

void DeviceModeManager::saveMode() {
    preferences.begin("mode", false);
    preferences.putUChar("device_mode", (uint8_t)currentMode);
    preferences.putBool("configured", true);
    preferences.end();
}

void DeviceModeManager::loadMode() {
    preferences.begin("mode", true);
    currentMode = (DeviceMode)preferences.getUChar("device_mode", MODE_BEACON);
    preferences.end();
}

bool DeviceModeManager::setBeaconConfig(const char* name, uint16_t intervalMs, int8_t txPower) {
    // Validate interval
    if (intervalMs < BEACON_MIN_INTERVAL_MS) intervalMs = BEACON_MIN_INTERVAL_MS;
    if (intervalMs > BEACON_MAX_INTERVAL_MS) intervalMs = BEACON_MAX_INTERVAL_MS;
    
    // Validate TX power (-12 to +9 dBm typical for ESP32)
    if (txPower < -12) txPower = -12;
    if (txPower > 9) txPower = 9;
    
    // Set configuration
    beaconConfig.intervalMs = intervalMs;
    beaconConfig.txPower = txPower;
    
    // Check if custom name or default (BT-beacon or empty triggers auto MAC suffix)
    // Empty string or "BT-beacon" = use default with MAC suffix
    // Any other string = use as custom name
    if (name && strlen(name) > 0 && strcmp(name, BEACON_DEFAULT_NAME) != 0) {
        // Custom name provided (not the default "BT-beacon")
        strncpy(beaconConfig.name, name, sizeof(beaconConfig.name) - 1);
        beaconConfig.name[sizeof(beaconConfig.name) - 1] = '\0';
        beaconConfig.useCustomName = true;
    } else {
        // Use default name (BT-beacon) with auto-generated MAC suffix
        strncpy(beaconConfig.name, BEACON_DEFAULT_NAME, sizeof(beaconConfig.name) - 1);
        beaconConfig.name[sizeof(beaconConfig.name) - 1] = '\0';
        beaconConfig.useCustomName = false;
    }
    
    saveBeaconConfig();
    return true;
}

void DeviceModeManager::resetBeaconConfigToDefault() {
    // Reset to neutral beacon mode with auto-generated name
    beaconConfig.intervalMs = BEACON_DEFAULT_INTERVAL_MS;
    beaconConfig.txPower = BEACON_DEFAULT_TX_POWER;
    beaconConfig.useCustomName = false;
    strncpy(beaconConfig.name, BEACON_DEFAULT_NAME, sizeof(beaconConfig.name) - 1);
    beaconConfig.name[sizeof(beaconConfig.name) - 1] = '\0';
    saveBeaconConfig();
}

void DeviceModeManager::saveBeaconConfig() {
    preferences.begin("beacon", false);
    preferences.putString("name", beaconConfig.name);
    preferences.putUShort("interval", beaconConfig.intervalMs);
    preferences.putChar("tx_power", beaconConfig.txPower);
    preferences.putBool("custom_name", beaconConfig.useCustomName);
    preferences.end();
}

void DeviceModeManager::loadBeaconConfig() {
    preferences.begin("beacon", true);
    
    String savedName = preferences.getString("name", BEACON_DEFAULT_NAME);
    strncpy(beaconConfig.name, savedName.c_str(), sizeof(beaconConfig.name) - 1);
    beaconConfig.name[sizeof(beaconConfig.name) - 1] = '\0';
    
    beaconConfig.intervalMs = preferences.getUShort("interval", BEACON_DEFAULT_INTERVAL_MS);
    beaconConfig.txPower = preferences.getChar("tx_power", BEACON_DEFAULT_TX_POWER);
    beaconConfig.useCustomName = preferences.getBool("custom_name", false);
    
    preferences.end();
    
    // Validate loaded values
    if (beaconConfig.intervalMs < BEACON_MIN_INTERVAL_MS) beaconConfig.intervalMs = BEACON_MIN_INTERVAL_MS;
    if (beaconConfig.intervalMs > BEACON_MAX_INTERVAL_MS) beaconConfig.intervalMs = BEACON_MAX_INTERVAL_MS;
}

String DeviceModeManager::generateBeaconNameFromMAC() {
    // Get BLE MAC address
    BLEAddress bleAddr = BLEDevice::getAddress();
    const uint8_t* bda = (const uint8_t*)bleAddr.getNative();
    
    // Create name with last 2 bytes of MAC
    char suffix[5];
    snprintf(suffix, sizeof(suffix), "%02X%02X", bda[4], bda[5]);
    
    String name = String(BEACON_DEFAULT_NAME) + String("_") + String(suffix);
    return name;
}

String DeviceModeManager::getBeaconName() {
    // Return the stored name without MAC suffix
    // BeaconManager will handle MAC suffix generation if needed
    return String(beaconConfig.name);
}

void DeviceModeManager::checkModeButton() {
    bool currentButtonState = !digitalRead(MODE_BUTTON_PIN);  // Pull-up inverted
    
    if (currentButtonState && !buttonPressed) {
        buttonPressed = true;
        buttonPressStart = millis();
    } else if (!currentButtonState && buttonPressed) {
        buttonPressed = false;
    }
}

bool DeviceModeManager::isModeButtonPressed() {
    if (buttonPressed && (millis() - buttonPressStart >= MODE_BUTTON_DURATION_MS)) {
        return true;
    }
    return false;
}

void DeviceModeManager::resetToDefaults() {
    setDefaults();
    currentMode = MODE_BEACON;
    isConfigured = false;
    
    preferences.begin("mode", false);
    preferences.clear();
    preferences.end();
    
    preferences.begin("beacon", false);
    preferences.clear();
    preferences.end();
}

void DeviceModeManager::clearAllSettings() {
    preferences.begin("mode", false);
    preferences.clear();
    preferences.end();
    
    preferences.begin("beacon", false);
    preferences.clear();
    preferences.end();
    
    isConfigured = false;
    currentMode = MODE_BEACON;
    setDefaults();
}
