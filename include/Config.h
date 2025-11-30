/**
 * @file Config.h
 * @brief Zentrale Konfigurationsdatei für alle Systemeinstellungen
 * 
 * Diese Datei enthält alle wichtigen Konstanten und Konfigurationen
 * für das BT Scanner Projekt. Später kann diese Konfiguration
 * von einer Online-Quelle abgerufen werden.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <HardwareSerial.h>

// =================== DEVICE MODE KONFIGURATION ===================
#define MODE_BUTTON_PIN 9  // Boot-Button (ESP32-C3 DevKitM-1) für Modus-Setup
#define MODE_BUTTON_DURATION_MS 4000  // 4 Sekunden halten für Setup-Portal

// Device-Betriebsmodi
enum DeviceMode {
    MODE_BEACON = 0,    // BLE-Beacon-Modus (default bei Erststart)
    MODE_SCANNER = 1    // BT-Scanner-Modus mit Web-Interface
};

// =================== BEACON MODE KONFIGURATION ===================
#define BEACON_DEFAULT_NAME "BT-beacon"  // Prefix für Beacon-Name (wird mit MAC erweitert)
#define BEACON_DEFAULT_INTERVAL_MS 800   // Default Sendeintervall in ms
#define BEACON_MIN_INTERVAL_MS 100       // Minimum 100ms (BLE-Spec: 20ms, aber praktisch)
#define BEACON_MAX_INTERVAL_MS 10000     // Maximum 10s
#define BEACON_DEFAULT_TX_POWER 0        // ESP_PWR_LVL_N0 (0dBm) = ~10m Reichweite
#define BEACON_LED_PULSE_MS 100          // LED-Puls Dauer bei jedem Beacon-Send
#define BEACON_ACTIVE_LOW 1              // LED invertierte Logik

// DEVELOPMENT: Deaktiviert Light Sleep für einfacheres Flashen
// Setze auf false für Produktion (20-40mA), true für Entwicklung (~80mA)
#define DISABLE_SLEEP false

// BLE TX Power Levels (ESP32 specific)
// ESP_PWR_LVL_N12 = -12dBm (~2m)
// ESP_PWR_LVL_N9  = -9dBm  (~3m)
// ESP_PWR_LVL_N6  = -6dBm  (~5m)
// ESP_PWR_LVL_N3  = -3dBm  (~7m)
// ESP_PWR_LVL_N0  =  0dBm  (~10m) <- DEFAULT
// ESP_PWR_LVL_P3  = +3dBm  (~15m)
// ESP_PWR_LVL_P6  = +6dBm  (~20m)
// ESP_PWR_LVL_P9  = +9dBm  (~30m)

// =================== WLAN KONFIGURATION ===================
// WiFiManager wird für initiale Konfiguration verwendet
// Diese Werte sind nur Fallback-Optionen
#define WIFI_SSID "mywifi"
#define WIFI_PASSWORD "mywifipw"
#define WIFI_TIMEOUT_MS 10000
#define WIFI_MANAGER_AP_NAME "ESP32-BT-Scanner"
// Kein Passwort für einfaches Setup!

// WiFiManager Optimierungen für Stabilität
#define WIFI_AP_CHANNEL 6        // Fester Kanal für Stabilität
#define WIFI_AP_MAX_CONNECTIONS 4 // Begrenzte Verbindungen

// =================== WATCHDOG KONFIGURATION ===================
#define WATCHDOG_TIMEOUT_SEC 30   // 30 Sekunden Watchdog-Timeout
#define WATCHDOG_ENABLED true     // Watchdog aktivieren

// =================== WEBSERVER KONFIGURATION ===================
#define WEBSERVER_PORT 80
#define MAX_CONCURRENT_CLIENTS 4

// =================== BLUETOOTH KONFIGURATION ===================
#define BT_SCAN_DURATION_SEC 2         // 2 Sekunden scannen
#define BT_SCAN_INTERVAL_MS 10000       // 10 Sekunden Gesamtzyklus (2s scan + 8s pause)
#define BT_SCAN_PAUSE_MS 8000           // 8 Sekunden Pause zwischen Scans
#define BT_MAX_FAILED_SCANS 3           // Maximale Anzahl fehlgeschlagener Scans
#define MAX_KNOWN_DEVICES 20
#define DEVICE_TIMEOUT_MS 120000        // 2 Minuten bis Gerät als "weg" gilt
#define DEFAULT_RSSI_THRESHOLD -80      // Standard RSSI-Grenzwert in dBm
#define MAX_COMMENT_LENGTH 32           // Maximale Kommentarlänge für bekannte Geräte

// =================== LED KONFIGURATION ===================
#define LED_BUILTIN_PIN 8
#define LED_BLINK_INTERVAL_MS 500
#define LED_ALERT_DURATION_MS 5000

// =================== RELAIS/OUTPUT KONFIGURATION ===================
#define RELAY_OUTPUT_PIN 4  // GPIO 4 für Torsteuerung/Relais

// =================== SYSTEM KONFIGURATION ===================
#define SERIAL_BAUD_RATE 115200
#define LOOP_DELAY_MS 100

// =================== DEBUG KONFIGURATION ===================
#define DEBUG_ENABLED false        // Produktionsversion: Debug deaktiviert
#define DEBUG_BLUETOOTH false      // Bluetooth Debug deaktiviert
#define DEBUG_WIFI false          // WiFi Debug deaktiviert  
#define DEBUG_WEBSERVER false     // WebServer Debug deaktiviert

// Debug Makros
#if DEBUG_ENABLED
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(format, ...) Serial.printf(format, ##__VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(format, ...)
#endif

// Bedingte Debug-Makros für Module
#if DEBUG_BLUETOOTH
  #define BT_DEBUG_PRINT(x) DEBUG_PRINT(x)
  #define BT_DEBUG_PRINTLN(x) DEBUG_PRINTLN(x)
  #define BT_DEBUG_PRINTF(format, ...) DEBUG_PRINTF(format, ##__VA_ARGS__)
#else
  #define BT_DEBUG_PRINT(x)
  #define BT_DEBUG_PRINTLN(x)
  #define BT_DEBUG_PRINTF(format, ...)
#endif

#if DEBUG_WIFI
  #define WIFI_DEBUG_PRINT(x) DEBUG_PRINT(x)
  #define WIFI_DEBUG_PRINTLN(x) DEBUG_PRINTLN(x)
  #define WIFI_DEBUG_PRINTF(format, ...) DEBUG_PRINTF(format, ##__VA_ARGS__)
#else
  #define WIFI_DEBUG_PRINT(x)
  #define WIFI_DEBUG_PRINTLN(x)
  #define WIFI_DEBUG_PRINTF(format, ...)
#endif

#if DEBUG_WEBSERVER
  #define WEB_DEBUG_PRINT(x) DEBUG_PRINT(x)
  #define WEB_DEBUG_PRINTLN(x) DEBUG_PRINTLN(x)
  #define WEB_DEBUG_PRINTF(format, ...) DEBUG_PRINTF(format, ##__VA_ARGS__)
#else
  #define WEB_DEBUG_PRINT(x)
  #define WEB_DEBUG_PRINTLN(x)
  #define WEB_DEBUG_PRINTF(format, ...)
#endif

#endif // CONFIG_H