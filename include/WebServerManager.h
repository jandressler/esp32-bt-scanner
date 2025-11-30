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
#include <DNSServer.h>
#include "Config.h"
#include "DeviceManager.h"
#include "BluetoothScanner.h"
#include "DeviceModeManager.h"

/**
 * @brief Klasse zur Verwaltung des Webservers
 */
class WebServerManager {
private:
    AsyncWebServer* server;
    AsyncWebServer* setupServer;
    DNSServer* dnsServer;
    DeviceManager* deviceManager;
    BluetoothScanner* bluetoothScanner;
    DeviceModeManager* modeManager;
    bool isRunning;
    bool setupServerStarted;
    bool isInSecureMode;
    bool setupComplete;
    
public:
    WebServerManager();
    ~WebServerManager();
    
    // Initialization
    bool begin(DeviceManager* devMgr, BluetoothScanner* btScanner);
    void end();
    
    // Setup
    void startSetupServer(const String& apPassword = "");
    void startSetupMode();
    bool isServerRunning() const { return isRunning; }
    bool isSetupRequired() const;
    bool isSetupComplete() const { return setupComplete; }
    
    // Configuration
    void setModeManager(DeviceModeManager* mgr) { modeManager = mgr; }
    
    // DNS Server handling for captive portal
    void processDNS();
    
private:
    void setupMainServerRoutes();
    void setupSetupRoutes();
    
    // API Handlers
    void handleStatusAPI(AsyncWebServerRequest *request);
    void handleDevicesAPI(AsyncWebServerRequest *request);
    
    // Device Management
    void handleSetKnownDevice(AsyncWebServerRequest *request);
    void handleStartScan(AsyncWebServerRequest *request);
    void handleClearDevices(AsyncWebServerRequest *request);
    
    // Export/Import
    void handleExportDevicesFile(AsyncWebServerRequest *request);
    void handleImportDevicesFile(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    
    // Setup WiFi
    void handleSetupRoot(AsyncWebServerRequest *request);
    void handleSetupWiFi(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleSetupAP(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleScanNetworks(AsyncWebServerRequest *request);
    
    // System Control
    void handleFactoryReset(AsyncWebServerRequest *request);
    void handleSystemReboot(AsyncWebServerRequest *request);
    void handleBluetoothReset(AsyncWebServerRequest *request);
    
    // Mode Setup
    void handleModeSetup(AsyncWebServerRequest *request);
    void handleBeaconConfigPage(AsyncWebServerRequest *request);
    void handleScannerConfigPage(AsyncWebServerRequest *request);
    void handleModeSelection(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleBeaconConfig(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    
    // Helpers
    String formatRelativeTime(unsigned long seconds);
    void updateAllDeviceStatus();
    void sendJSONResponse(AsyncWebServerRequest *request, const String& status, const String& message = "");
};

#endif // WEBSERVER_MANAGER_H
