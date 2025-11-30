/**
 * @file BeaconManager.cpp
 * @brief Implementation des BLE-Beacon-Managers
 */

#include "BeaconManager.h"
#include <esp_system.h>
#include <esp_pm.h>
#include <esp_sleep.h>
#include <WiFi.h>
#include <esp_bt_main.h>

BeaconManager::BeaconManager() 
    : advertising(nullptr), intervalMs(BEACON_DEFAULT_INTERVAL_MS), 
      txPower(BEACON_DEFAULT_TX_POWER), isRunning(false), lastAdvertiseTime(0) {
}

BeaconManager::~BeaconManager() {
    end();
}

bool BeaconManager::begin(const String& name, uint16_t interval, int8_t power) {
    intervalMs = interval;
    txPower = power;
    
    // Clean shutdown of any existing BLE
    BLEDevice::deinit(true);
    delay(100);
    
    // Determine the final beacon name
    // Check if this is the default name (needs MAC suffix)
    bool needsMacSuffix = (name.length() == 0 || name == "BT-beacon");
    
    if (needsMacSuffix) {
        // For default name, get BLE MAC directly from hardware
        uint8_t bleMac[6];
        esp_read_mac(bleMac, ESP_MAC_BT);
        char suffix[5];
        snprintf(suffix, sizeof(suffix), "%02X%02X", bleMac[4], bleMac[5]);
        beaconName = String("BT-beacon_") + String(suffix);
    } else {
        // Custom name
        beaconName = name;
    }
    
    // Disable WiFi explicitly to save power
    WiFi.mode(WIFI_OFF);
    delay(10);
    
    // Init BLE with the beacon name
    // Note: BLE controller caches the device name in flash.
    // The advertised data will contain the correct name,
    // but the scan name requires a power-cycle to update.
    BLEDevice::init(beaconName.c_str());
    
    // Enable BLE sleep mode (BLE controller sleeps between advertising)
    esp_bt_sleep_enable();
    
    // Configure power management for light sleep with BLE
    esp_pm_config_esp32c3_t pm_config;
    pm_config.max_freq_mhz = 80;  // 80 MHz when active
    pm_config.min_freq_mhz = 10;  // 10 MHz when idle
    pm_config.light_sleep_enable = true;  // Enable automatic light sleep
    esp_pm_configure(&pm_config);
    
    // Lower CPU frequency base to 80 MHz
    setCpuFrequencyMhz(80);
    
    advertising = BLEDevice::getAdvertising();
    
    if (!advertising) {
        return false;
    }
    
    // Set BLE TX Power
    esp_power_level_t powerLevel = dbmToESPPowerLevel(txPower);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, powerLevel);
    
    // Configure advertising data
    configureAdvertising();
    
    // Set advertising interval
    uint16_t advUnits = intervalToAdvUnits(intervalMs);
    advertising->setMinInterval(advUnits);
    advertising->setMaxInterval(advUnits);
    
    isRunning = true;
    lastAdvertiseTime = millis();
    
    return true;
}

void BeaconManager::end() {
    if (advertising) {
        advertising->stop();
        advertising = nullptr;
    }
    BLEDevice::deinit(true);  // Force full deinit to clear cached name
    isRunning = false;
}

void BeaconManager::start() {
    if (advertising) {
        // Don't start advertising here - loop() controls duty cycle
        advertising->stop();
        isRunning = true;
        lastAdvertiseTime = millis();
    }
}

void BeaconManager::stop() {
    if (advertising && isRunning) {
        advertising->stop();
        isRunning = false;
    }
}

void BeaconManager::loop() {
    if (!isRunning || !advertising) return;
    
    // Calculate sleep time: interval minus advertising window (100ms)
    uint32_t sleepMs = (intervalMs > 100) ? (intervalMs - 100) : intervalMs;
    
    // Enter true light sleep to save power (unless disabled for development)
    #if !DISABLE_SLEEP
    esp_sleep_enable_timer_wakeup((uint64_t)sleepMs * 1000ULL);
    esp_light_sleep_start();
    
    // Small stabilization delay after wakeup
    delay(5);
    #else
    // Development mode: just delay instead of sleep
    delay(sleepMs);
    #endif
    
    // Start advertising
    advertising->start();
    delay(10);  // Allow advertising to queue
    
    // LED on during advertising window
    blinkLED(100);
    
    // Stop advertising until next cycle
    advertising->stop();
}

void BeaconManager::setName(const String& name) {
    beaconName = name;
    configureAdvertising();
}

void BeaconManager::setInterval(uint16_t interval) {
    intervalMs = interval;
    if (advertising) {
        uint16_t advUnits = intervalToAdvUnits(intervalMs);
        advertising->setMinInterval(advUnits);
        advertising->setMaxInterval(advUnits);
    }
}

void BeaconManager::setTXPower(int8_t power) {
    txPower = power;
    esp_power_level_t powerLevel = dbmToESPPowerLevel(txPower);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, powerLevel);
}

uint16_t BeaconManager::intervalToAdvUnits(uint16_t ms) {
    // BLE advertising interval units are 0.625ms
    // Allowed range: 20ms (0x0020) to 10240ms (0x4000)
    uint32_t minMs = 20;
    uint32_t maxMs = 10240;
    uint32_t clamped = (ms < minMs) ? minMs : (ms > maxMs ? maxMs : ms);
    
    // Convert to units (0.625ms per unit)
    uint32_t units = (uint32_t)((clamped * 1000) / 625);
    if (units < 0x0020) units = 0x0020;
    if (units > 0x4000) units = 0x4000;
    
    return (uint16_t)units;
}

esp_power_level_t BeaconManager::dbmToESPPowerLevel(int8_t dbm) {
    // Map dBm to ESP32 power levels
    if (dbm <= -12) return ESP_PWR_LVL_N12;      // -12 dBm
    else if (dbm <= -9) return ESP_PWR_LVL_N9;   // -9 dBm
    else if (dbm <= -6) return ESP_PWR_LVL_N6;   // -6 dBm
    else if (dbm <= -3) return ESP_PWR_LVL_N3;   // -3 dBm
    else if (dbm <= 0) return ESP_PWR_LVL_N0;    // 0 dBm (default)
    else if (dbm <= 3) return ESP_PWR_LVL_P3;    // +3 dBm
    else if (dbm <= 6) return ESP_PWR_LVL_P6;    // +6 dBm
    else return ESP_PWR_LVL_P9;                   // +9 dBm
}

void BeaconManager::blinkLED(uint16_t durationMs) {
    // LED on (inverted logic: LOW = ON)
    if (BEACON_ACTIVE_LOW) {
        digitalWrite(LED_BUILTIN_PIN, LOW);
    } else {
        digitalWrite(LED_BUILTIN_PIN, HIGH);
    }
    
    delay(durationMs);
    
    // LED off
    if (BEACON_ACTIVE_LOW) {
        digitalWrite(LED_BUILTIN_PIN, HIGH);
    } else {
        digitalWrite(LED_BUILTIN_PIN, LOW);
    }
}

void BeaconManager::configureAdvertising() {
    if (!advertising) return;
    
    // Set advertising data
    BLEAdvertisementData advData;
    advData.setFlags(0x06);  // LE General Discoverable Mode, BR/EDR Not Supported
    advData.setName(beaconName.c_str());
    
    // Set scan response data
    BLEAdvertisementData scanResp;
    scanResp.setName(beaconName.c_str());
    
    advertising->setAdvertisementData(advData);
    advertising->setScanResponseData(scanResp);
    
    // Also set GAP device name
    esp_ble_gap_set_device_name(beaconName.c_str());
}
