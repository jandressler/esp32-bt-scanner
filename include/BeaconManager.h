/**
 * @file BeaconManager.h
 * @brief BLE-Beacon-Verwaltung für ESP32-C3
 * 
 * Diese Klasse implementiert einen stromsparenden BLE-Beacon-Modus
 * mit konfigurierbarem Namen, Sendeintervall und TX-Power.
 */

#ifndef BEACON_MANAGER_H
#define BEACON_MANAGER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEAdvertising.h>
#include <esp_gap_ble_api.h>
#include <esp_bt.h>
#include "Config.h"
#include "DeviceModeManager.h"

/**
 * @brief Beacon Manager Klasse
 */
class BeaconManager {
private:
    BLEAdvertising* advertising;
    String beaconName;
    uint16_t intervalMs;
    int8_t txPower;
    bool isRunning;
    unsigned long lastAdvertiseTime;
    
public:
    BeaconManager();
    ~BeaconManager();
    
    // Initialization
    bool begin(const String& name, uint16_t interval, int8_t power);
    void end();
    
    // Beacon Control
    void start();
    void stop();
    void loop();  // Für Beacon-Zyklus (senden, LED, sleep)
    
    // Configuration
    void setName(const String& name);
    void setInterval(uint16_t intervalMs);
    void setTXPower(int8_t power);
    
    // Status
    bool isActive() const { return isRunning; }
    String getName() const { return beaconName; }
    
private:
    uint16_t intervalToAdvUnits(uint16_t ms);
    esp_power_level_t dbmToESPPowerLevel(int8_t dbm);
    void blinkLED(uint16_t durationMs);
    void configureAdvertising();
};

#endif // BEACON_MANAGER_H
