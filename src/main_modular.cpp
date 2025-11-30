/**
 * @file main.cpp
 * @brief Hauptprogramm des ESP32-C3 Bluetooth Scanners / Beacons
 * 
 * Dieses Programm implementiert zwei Betriebsmodi:
 * 1. BEACON: Stromsparender BLE-Beacon ohne WiFi
 * 2. SCANNER: Bluetooth-Scanner mit Web-Interface und WiFi-Management
 * 
 * Modusauswahl: Boot-Button >3s drücken → Setup-Portal
 * Standard bei Erststart: BEACON-Modus
 */

#include <Arduino.h>
#include <esp_task_wdt.h>
#include <BLEDevice.h>
#include "Config.h"
#include "DeviceModeManager.h"
#include "BeaconManager.h"
#include "DeviceManager.h"
#include "BluetoothScanner.h"
#include "WiFiManager.h"
#include "WebServerManager.h"

// Global instances
DeviceModeManager modeManager;
BeaconManager beaconManager;
DeviceManager deviceManager;
BluetoothScanner bluetoothScanner;
WiFiManager wifiManager;
WebServerManager webServerManager;

// Global device array (nur für Scanner-Modus)
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
void enterSetupPortal();

void setup() {
    // Initialize GPIO (common for both modes)
    pinMode(LED_BUILTIN_PIN, OUTPUT);
    pinMode(RELAY_OUTPUT_PIN, OUTPUT);
    digitalWrite(LED_BUILTIN_PIN, HIGH);  // LED OFF initially
    digitalWrite(RELAY_OUTPUT_PIN, LOW);   // Relay OFF initially
    
    startTime = millis();
    
    // Initialize Mode Manager (determines Beacon vs Scanner)
    if (!modeManager.begin()) {
        // Fatal error - blink LED rapidly
        for (int i = 0; i < 10; i++) {
            digitalWrite(LED_BUILTIN_PIN, i % 2);
            delay(100);
        }
        ESP.restart();
    }
    
    // Check if boot button is pressed for mode setup
    // Button must be held for full 4s to enter setup mode (safety feature)
    // LED turns ON after 4s to indicate setup mode is about to activate
    bool setupModeRequested = false;
    unsigned long buttonPressStart = 0;
    bool buttonWasPressed = false;
    
    // Check if button is pressed at boot (100ms detection window)
    for (int i = 0; i < 10; i++) {
        if (digitalRead(MODE_BUTTON_PIN) == LOW) {
            buttonWasPressed = true;
            buttonPressStart = millis();
            break;
        }
        delay(10);
    }
    
    // If button detected, monitor hold duration
    if (buttonWasPressed) {
        while (digitalRead(MODE_BUTTON_PIN) == LOW) {
            unsigned long holdTime = millis() - buttonPressStart;
            
            // After 4s: Enter setup mode
            if (holdTime >= MODE_BUTTON_DURATION_MS) {
                setupModeRequested = true;
                
                // Visual feedback: Fast blinking indicates setup mode starting
                for (int i = 0; i < 20; i++) {
                    digitalWrite(LED_BUILTIN_PIN, i % 2 ? LOW : HIGH);
                    delay(100);
                }
                break;
            }
            
            delay(10);
        }
    }
    
    // Enter setup portal if button was held long enough
    if (setupModeRequested) {
        enterSetupPortal();
        return;  // Setup portal handles everything, then restarts
    }
    
    // ============ BEACON MODE ============
    if (modeManager.isInBeaconMode()) {
        // Beacon mode: No WiFi, minimal power, just BLE beacon
        BeaconConfig config = modeManager.getBeaconConfig();
        String beaconName = modeManager.getBeaconName();
        
        // Fresh boot - BLE is clean, just init with the configured name
        if (!beaconManager.begin(beaconName, config.intervalMs, config.txPower)) {
            // Beacon init failed - restart
            delay(1000);
            ESP.restart();
        }
        
        // Beacon-Start Feedback: 1 langes Blinken (1s)
        digitalWrite(LED_BUILTIN_PIN, LOW);  // LED AN
        delay(1000);
        digitalWrite(LED_BUILTIN_PIN, HIGH); // LED AUS
        delay(100);
        
        beaconManager.start();
        systemInitialized = true;
        ledInitialized = true;
        
        return;  // Skip scanner initialization
    }
    
    // ============ SCANNER MODE ============
    // Hardware Watchdog initialize (scanner mode only)
    initializeWatchdog();
    
    // Initialize device arrays
    memset(devices, 0, sizeof(devices));
    
    // Initialize modules
    deviceManager.begin(devices, MAX_DEVICES);
    
    if (!wifiManager.begin()) {
        // WiFi init failed - continue anyway
    }
    
    webServerManager.setModeManager(&modeManager);  // Pass mode manager for config
    if (!webServerManager.begin(&deviceManager, &bluetoothScanner)) {
        // Web server init failed
    }
    
    // Initialize Bluetooth only in secure mode
    if (wifiManager.isSecure()) {
        if (!bluetoothScanner.begin(&deviceManager)) {
            // BLE init failed
        }
    }
    
    systemInitialized = true;
    ledInitialized = true;
    setPresenceOutput(false);
    knownDevicesPresent = false;
}

void loop() {
    // ============ BEACON MODE LOOP ============
    if (modeManager.isInBeaconMode()) {
        // Simple beacon loop: advertise, blink LED, sleep
        beaconManager.loop();
        
        // Check for mode button press (for potential reconfiguration)
        modeManager.checkModeButton();
        if (modeManager.isModeButtonPressed()) {
            // User wants to reconfigure - restart to setup portal
            ESP.restart();
        }
        
        return;  // No further processing needed in beacon mode
    }
    
    // ============ SCANNER MODE LOOP ============
    static unsigned long lastDebugPrint = 0;
    unsigned long now = millis();
    
    // Debug output every 30 seconds
    if (now - lastDebugPrint > 30000) {
        lastDebugPrint = now;
        
        if (bluetoothScanner.isInitialized()) {
            // Debug output removed for production
        }
    }
    
    // Feed watchdog (prevents automatic restart)
    feedWatchdog();
    
    // WiFi manager loop (DNS processing for captive portal)
    wifiManager.loop();
    
    // Mode button monitoring (for reconfiguration)
    modeManager.checkModeButton();
    if (modeManager.isModeButtonPressed()) {
        // User wants to reconfigure - enter setup portal
        ESP.restart();
    }
    
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

void enterSetupPortal() {
    // Setup Portal: Allow user to choose mode and configure device
    // Läuft bis zur Konfiguration (kein Timeout!)
    
    // Watchdog deaktivieren für Setup-Portal
    if (WATCHDOG_ENABLED) {
        esp_task_wdt_delete(NULL);
        esp_task_wdt_deinit();
    }
    
    // Stop any existing WiFi
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(100);
    
    // Start WiFi AP with fixed IP configuration
    WiFi.mode(WIFI_AP);
    
    // Configure AP with fixed IP settings
    IPAddress local_IP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    
    WiFi.softAPConfig(local_IP, gateway, subnet);
    
    // Start AP without password for easy access
    bool apStarted = WiFi.softAP(WIFI_MANAGER_AP_NAME);
    
    if (!apStarted) {
        // AP failed to start - rapid blink and restart
        for (int i = 0; i < 20; i++) {
            digitalWrite(LED_BUILTIN_PIN, i % 2 ? LOW : HIGH);
            delay(100);
        }
        ESP.restart();
    }
    
    delay(1000);  // Give AP time to fully initialize
    
    // Verify IP configuration
    IPAddress myIP = WiFi.softAPIP();
    
    // No BLE during setup - keeps it simple and avoids reinit issues
    
    // LED-Feedback: Setup-Modus aktiv (sehr schnelles Blinken 100ms/100ms)
    unsigned long lastBlink = 0;
    bool ledState = false;
    
    // Start web server for mode selection
    webServerManager.setModeManager(&modeManager);
    webServerManager.begin(&deviceManager, &bluetoothScanner);
    webServerManager.startSetupMode();  // Special setup mode
    
    // Run setup portal until configuration is complete (NO TIMEOUT)
    bool configured = false;
    unsigned long lastStatusCheck = 0;
    
    while (!configured) {
        // LED-Feedback: Sehr schnelles Blinken alle 100ms (Setup-Modus)
        unsigned long now = millis();
        if (now - lastBlink > 100) {
            lastBlink = now;
            ledState = !ledState;
            digitalWrite(LED_BUILTIN_PIN, ledState ? LOW : HIGH);
        }
        
        // Process DNS requests for captive portal
        webServerManager.processDNS();
        
        // Keep neutral beacon running during setup
        beaconManager.loop();
        
        // Check AP status periodically
        if (now - lastStatusCheck > 5000) {
            lastStatusCheck = now;
            int clients = WiFi.softAPgetStationNum();
            // Status check - could add LED pattern for connected clients
        }
        
        // Check if user completed setup
        if (webServerManager.isSetupComplete()) {
            configured = true;
            break;
        }
        
        // Small yield for system tasks
        delay(10);
        yield();
    }
    
    // Configuration complete - stop neutral beacon and restart
    beaconManager.stop();
    beaconManager.end();
    
    // Visual feedback
    for (int i = 0; i < 10; i++) {
        digitalWrite(LED_BUILTIN_PIN, i % 2 ? LOW : HIGH);
        delay(100);
    }
    delay(1000);
    ESP.restart();
}
