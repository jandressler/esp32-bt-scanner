/**
 * @file WebServerManager.h
 * @brief HTTP-Webserver Management
 * 
 * Diese Klasse verwaltet den HTTP-Webserver und stellt
 * eine Web-Interface für die Bluetooth-Scanner Anwendung bereit.
 */

#ifndef WEBSERVER_MANAGER_H
#define WEBSERVER_MANAGER_H

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <Preferences.h>
#include "Config.h"
#include "DeviceManager.h"
#include "BluetoothScanner.h"

/**
 * @brief Klasse zur Verwaltung des Webservers
 */
class WebServerManager {
private:
    AsyncWebServer* server;
    AsyncWebServer* setupServer;
    DeviceManager* deviceManager;
    BluetoothScanner* bluetoothScanner;
    bool isRunning;
    bool setupServerStarted;
    bool isInSecureMode;
    
public:
    /**
     * @brief Konstruktor
     */
    WebServerManager();
    
    /**
     * @brief Destruktor
     */
    ~WebServerManager();
    
    /**
     * @brief Initialisierung des Webservers
     * @param devMgr Zeiger auf DeviceManager
     * @param btScanner Zeiger auf BluetoothScanner
     * @return true wenn erfolgreich
     */
    bool begin(DeviceManager* devMgr, BluetoothScanner* btScanner);
    
    /**
     * @brief Webserver stoppen
     */
    void end();
    
    /**
     * @brief Setup-Server für WiFi-Konfiguration starten
     * @param apPassword AP-Passwort (leer für offenen AP)
     */
    void startSetupServer(const String& apPassword = "");
    
    /**
     * @brief Prüft ob der Server läuft
     * @return true wenn Server läuft
     */
    bool isServerRunning() const { return isRunning; }
    
    /**
     * @brief Setzt den Sicherheitsmodus
     * @param secure true für sicheren Modus
     */
    void setSecureMode(bool secure) { isInSecureMode = secure; }
    
    /**
     * @brief Prüft ob Setup erforderlich ist
     * @return true wenn Setup erforderlich
     */
    bool isSetupRequired() const;
    
private:
    void setupMainServerRoutes();
    void setupAPIRoutes();
    void setupLoxoneRoutes();
    void setupSetupRoutes();
    void handleStatusAPI(AsyncWebServerRequest *request);
    void handleDevicesAPI(AsyncWebServerRequest *request);
    void handleAddKnownAPI(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleRemoveKnownAPI(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    
    // Device Management Handlers
    void handleSetKnownDevice(AsyncWebServerRequest *request);
    void handleStartScan(AsyncWebServerRequest *request);
    void handleClearDevices(AsyncWebServerRequest *request);
    
    // Export/Import Handlers
    void handleExportDevicesFile(AsyncWebServerRequest *request);
    void handleImportDevicesFile(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    
    // Setup-spezifische Handler
    void handleSetupRoot(AsyncWebServerRequest *request);
    void handleSetupWiFi(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleSetupAP(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleScanNetworks(AsyncWebServerRequest *request);
    void handleWiFiReset(AsyncWebServerRequest *request);
    void handleSystemReset(AsyncWebServerRequest *request);
    void handleBluetoothReset(AsyncWebServerRequest *request);
    
    // Hilfs-Methoden
    String formatRelativeTime(unsigned long seconds);
    void updateAllDeviceStatus();
    String generateSetupHTML();
    String generateMainHTML();
    String generateMainHTMLScript();
    void sendJSONResponse(AsyncWebServerRequest *request, const String& status, const String& message = "");
};

#endif // WEBSERVER_MANAGER_H