/**
 * @file main.cpp
 * @brief Hauptprogramm des ESP32-C3 Bluetooth Scanners
 * 
 * Dieses Programm implementiert einen Bluetooth-Scanner mit Web-Interface,
 * WiFi-Management und Geräte-Persistierung in modularer Architektur.
 */

#include <Arduino.h>
#include <esp_task_wdt.h>
#include "Config.h"
#include "DeviceManager.h"
#include "BluetoothScanner.h"
#include "WiFiManager.h"
#include "WebServerManager.h"

// Global instances
DeviceManager deviceManager;
BluetoothScanner bluetoothScanner;
WiFiManager wifiManager;
WebServerManager webServerManager;

// Global device array
SafeDevice devices[MAX_DEVICES];

// Global state
bool systemInitialized = false;
bool knownDevicesPresent = false;
bool ledInitialized = false;
unsigned long startTime = 0;

// Function declarations
void initializeWatchdog();
void feedWatchdog();
void setPresenceOutput(bool devicePresent);
void setPresenceOutput(bool devicePresent, const char* triggerDevice, const char* triggerName, const char* triggerComment);
void updateLEDStatus();

void setup() {
    // Initialize serial for debugging
    
    
    // Hardware Watchdog initialize (must be first!)
    initializeWatchdog();
    
    // Initialize GPIO
    pinMode(LED_BUILTIN_PIN, OUTPUT);
    pinMode(RELAY_OUTPUT_PIN, OUTPUT);
    
    // Both outputs initially OFF
    setPresenceOutput(false);
    
    startTime = millis();
    
    // Initialize device arrays
    memset(devices, 0, sizeof(devices));
    
    // Initialize modules
    
    deviceManager.begin(devices, MAX_DEVICES);
    
    
    if (!wifiManager.begin()) {
        
    }
    
    
    webServerManager.setSecureMode(wifiManager.isSecure());
    if (!webServerManager.begin(&deviceManager, &bluetoothScanner)) {
        
    }
    
    // Initialize Bluetooth only in secure mode
    if (wifiManager.isSecure()) {
        
        if (!bluetoothScanner.begin(&deviceManager)) {
            
        }
    } else {
        
    }
    
    systemInitialized = true;
    ledInitialized = true;
    setPresenceOutput(false);
    knownDevicesPresent = false;
    
    
    if (wifiManager.isConnected()) {
        
    } else if (wifiManager.isInAPMode()) {
        
    } else {
        
    }
}

void loop() {
    static unsigned long lastDebugPrint = 0;
    unsigned long now = millis();
    
    // Debug output every 30 seconds
    if (now - lastDebugPrint > 30000) {
        lastDebugPrint = now;
        
        if (bluetoothScanner.isInitialized()) {
            
        }
    }
    
    // Feed watchdog (prevents automatic restart)
    feedWatchdog();
    
    // WiFi manager loop (DNS processing for captive portal)
    wifiManager.loop();
    
    // WiFi reset button monitoring
    wifiManager.checkResetButton();
    
    // Bluetooth scanning only in secure mode
    if (systemInitialized && wifiManager.isSecure() && bluetoothScanner.isInitialized()) {
        bluetoothScanner.performAutomaticScanCycle();  // Automatischer 2s/8s Zyklus
        updateLEDStatus();
    }
    
    delay(100); // Small pause
}

void initializeWatchdog() {
    if (WATCHDOG_ENABLED) {
        esp_task_wdt_init(WATCHDOG_TIMEOUT_SEC, true);
        esp_task_wdt_add(NULL);
        
    }
}

void feedWatchdog() {
    if (WATCHDOG_ENABLED) {
        esp_task_wdt_reset();
    }
}

// Simple version without logging (for setup/reset calls)
void setPresenceOutput(bool devicePresent) {
    if (!ledInitialized) return;
    
    // LED: inverted logic (LOW = ON, HIGH = OFF)
    digitalWrite(LED_BUILTIN_PIN, devicePresent ? LOW : HIGH);
    
    // Relay: normal logic (HIGH = ON, LOW = OFF)
    digitalWrite(RELAY_OUTPUT_PIN, devicePresent ? HIGH : LOW);
}

// Full version with logging (for device detection calls)
void setPresenceOutput(bool devicePresent, const char* triggerDevice, const char* triggerName, const char* triggerComment) {
    if (!ledInitialized) return;
    
    // Nur loggen wenn sich der Status ändert
    static bool lastOutputState = false;
    if (devicePresent != lastOutputState) {
        lastOutputState = devicePresent;
        
        // Log-Eintrag erstellen
        String reason;
        if (devicePresent) {
            reason = "Gerät erkannt";
        } else {
            reason = "Kein bekanntes Gerät in Reichweite";
        }
        
        deviceManager.logOutputChange(
            triggerDevice,
            triggerName,
            triggerComment,
            devicePresent,
            reason.c_str()
        );
    }
    
    // LED: inverted logic (LOW = ON, HIGH = OFF)
    digitalWrite(LED_BUILTIN_PIN, devicePresent ? LOW : HIGH);
    
    // Relay: normal logic (HIGH = ON, LOW = OFF)
    digitalWrite(RELAY_OUTPUT_PIN, devicePresent ? HIGH : LOW);
}

void updateLEDStatus() {
    if (!ledInitialized) return;
    
    bool anyKnownPresent = false;
    SafeDevice* devices = deviceManager.getDevices();
    int deviceCount = deviceManager.getDeviceCount();
    
    // Finde das Gerät das die Schaltung verursacht
    const char* triggerDevice = "";
    const char* triggerName = "";
    const char* triggerComment = "";
    
    // LED nur AN wenn bekanntes Gerät im grünen Proximity-Bereich ist
    for (int i = 0; i < deviceCount; i++) {
        if (devices[i].isActive && devices[i].isKnown) {
            // Prüfe ob Gerät nah genug ist (grüner Proximity)
            if (devices[i].rssi >= devices[i].rssiThreshold) {
                anyKnownPresent = true;
                triggerDevice = devices[i].address;
                triggerName = devices[i].name;
                triggerComment = devices[i].comment;
                break;  // Erstes gefundenes Gerät verwenden
            }
        }
    }
    
    knownDevicesPresent = anyKnownPresent;
    setPresenceOutput(anyKnownPresent, triggerDevice, triggerName, triggerComment);
}