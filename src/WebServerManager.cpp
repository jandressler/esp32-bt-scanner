#include "WebServerManager.h"
#include "Config.h"
#include "WiFiManager.h"
#include "WebUI.h"
#include <BLEDevice.h>
#include <esp_system.h>

extern WiFiManager wifiManager;

WebServerManager::WebServerManager() : server(nullptr), setupServer(nullptr), dnsServer(nullptr), deviceManager(nullptr), bluetoothScanner(nullptr), modeManager(nullptr), isRunning(false), setupServerStarted(false), isInSecureMode(false), setupComplete(false) {}

WebServerManager::~WebServerManager() { end(); }

bool WebServerManager::begin(DeviceManager* devMgr, BluetoothScanner* btScanner) {
    if (!devMgr || !btScanner) return false;
    deviceManager = devMgr;
    bluetoothScanner = btScanner;
    if (!server) server = new AsyncWebServer(80);
    
    // Setup-Interface wenn nicht sicher, sonst Hauptinterface
    if (isSetupRequired()) {
        setupSetupRoutes();
    } else {
        setupMainServerRoutes();
    }
    
    server->begin();
    isRunning = true;
    return true;
}

void WebServerManager::end() {
    if (dnsServer) { dnsServer->stop(); delete dnsServer; dnsServer = nullptr; }
    if (server) { server->end(); delete server; server = nullptr; }
    if (setupServer) { setupServer->end(); delete setupServer; setupServer = nullptr; }
    isRunning = false;
    setupServerStarted = false;
}

bool WebServerManager::isSetupRequired() const {
    return !wifiManager.isSecure();
}

void WebServerManager::startSetupServer(const String& apPassword) {
    // Nicht mehr benötigt - wird direkt in begin() gehandhabt
}

void WebServerManager::setupSetupRoutes() {
    if (!server) return;
    server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
        handleModeSetup(request);
    });
    server->on("/api/scan", HTTP_GET, [this](AsyncWebServerRequest *request){
        handleScanNetworks(request);
    });
    // Setup-spezifische Handler mit Body-Parsing
    server->on("/setup/wifi", HTTP_POST, [this](AsyncWebServerRequest *request){
        sendJSONResponse(request, "success", "WLAN wird verbunden...");
    }, nullptr, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
        handleSetupWiFi(request, data, len, index, total);
    });
    server->on("/setup/ap", HTTP_POST, [this](AsyncWebServerRequest *request){
        sendJSONResponse(request, "success", "AP wird eingerichtet...");
    }, nullptr, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
        handleSetupAP(request, data, len, index, total);
    });
}

void WebServerManager::setupMainServerRoutes() {
    if (!server) return;
    
    server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
        // Dynamisch je nach aktuellem Sicherheitsstatus Setup- oder Hauptseite liefern
        if (isSetupRequired()) {
            handleModeSetup(request);
            return;
        }
        String page = WebUI::generateMainHTML();
        request->send(200, "text/html", page);
    });
    
    server->on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request){
        handleStatusAPI(request);
    });
    
    server->on("/api/devices", HTTP_GET, [this](AsyncWebServerRequest *request){
        handleDevicesAPI(request);
    });
    
    server->on("/api/output-log", HTTP_GET, [this](AsyncWebServerRequest *request){
        String logJson = deviceManager->getOutputLogJson();
        request->send(200, "application/json", logJson);
    });
    
    server->on("/api/output-log/clear", HTTP_POST, [this](AsyncWebServerRequest *request){
        deviceManager->clearOutputLog();
        sendJSONResponse(request, "success", "Output-Log gelöscht");
    });
    // Test-Endpoint für Output-Log
    server->on("/api/output-log/test", HTTP_POST, [this](AsyncWebServerRequest *request){
        deviceManager->logOutputChange("", "System", "Test", true, "Test-Logeintrag erstellt");
        sendJSONResponse(request, "success", "Test-Logeintrag erstellt");
    });
    
    server->on("/api/wifi/reset", HTTP_POST, [this](AsyncWebServerRequest *request){
        handleWiFiReset(request);
    });
    
    server->on("/api/system/reset", HTTP_POST, [this](AsyncWebServerRequest *request){
        handleSystemReset(request);
    });

    // Health check endpoint
    server->on("/health", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "ok");
    });

    // Redirect unknown paths to root to avoid blank pages
    server->onNotFound([](AsyncWebServerRequest *request){
        request->redirect("/");
    });
    
    server->on("/api/bluetooth/reset", HTTP_POST, [this](AsyncWebServerRequest *request){
        handleBluetoothReset(request);
    });
    
    // BLE Device Management API
    server->on("/api/device/known", HTTP_POST, [this](AsyncWebServerRequest *request){
        handleSetKnownDevice(request);
    });

    // Export/Import API
    server->on("/api/export-devices-file", HTTP_GET, [this](AsyncWebServerRequest *request){
        handleExportDevicesFile(request);
    });
    
    server->on("/api/import-devices", HTTP_POST, [this](AsyncWebServerRequest *request){
        // Response wird im body handler gesendet
    }, nullptr, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
        handleImportDevicesFile(request, data, len, index, total);
    });

    // Loxone API Endpunkte
    server->on("/loxone/presence", HTTP_GET, [this](AsyncWebServerRequest *request){
        // Text: "present" oder "absent"
        bool present = false;
        SafeDevice* devices = deviceManager->getDevices();
        int knownCount = deviceManager->getKnownCount();
        int* thresholds = deviceManager->getKnownRSSIThresholds();
        char (*macs)[18] = deviceManager->getKnownMACs();
        for (int i = 0; i < knownCount; i++) {
            for (int j = 0; j < deviceManager->getDeviceCount(); j++) {
                if (strcmp(devices[j].address, macs[i]) == 0 && devices[j].isActive && devices[j].rssi >= thresholds[i]) {
                    present = true;
                    break;
                }
            }
            if (present) break;
        }
        request->send(200, "text/plain", present ? "present" : "absent");
    });

    server->on("/loxone/presence_num", HTTP_GET, [this](AsyncWebServerRequest *request){
        // Numeric: 1 = present, 0 = absent
        bool present = false;
        SafeDevice* devices = deviceManager->getDevices();
        int knownCount = deviceManager->getKnownCount();
        int* thresholds = deviceManager->getKnownRSSIThresholds();
        char (*macs)[18] = deviceManager->getKnownMACs();
        for (int i = 0; i < knownCount; i++) {
            for (int j = 0; j < deviceManager->getDeviceCount(); j++) {
                if (strcmp(devices[j].address, macs[i]) == 0 && devices[j].isActive && devices[j].rssi >= thresholds[i]) {
                    present = true;
                    break;
                }
            }
            if (present) break;
        }
        request->send(200, "text/plain", present ? "1" : "0");
    });

    server->on("/loxone/status", HTTP_GET, [this](AsyncWebServerRequest *request){
        // Text: "online" oder "error"
        bool ok = wifiManager.isConnected();
        request->send(200, "text/plain", ok ? "online" : "error");
    });

    server->on("/loxone/status_num", HTTP_GET, [this](AsyncWebServerRequest *request){
        // Numeric: 1 = online, 0 = error
        bool ok = wifiManager.isConnected();
        request->send(200, "text/plain", ok ? "1" : "0");
    });

    server->on("/loxone/device", HTTP_GET, [this](AsyncWebServerRequest *request){
        // Text: "present", "absent", "unknown"
        if (!request->hasParam("address")) {
            request->send(400, "text/plain", "unknown");
            return;
        }
        String address = request->getParam("address")->value();
        SafeDevice* devices = deviceManager->getDevices();
        int found = -1;
        for (int i = 0; i < deviceManager->getDeviceCount(); i++) {
            if (address.equalsIgnoreCase(devices[i].address)) {
                found = i;
                break;
            }
        }
        if (found == -1) {
            request->send(200, "text/plain", "unknown");
            return;
        }
        bool present = devices[found].isActive && devices[found].isKnown && devices[found].rssi >= devices[found].rssiThreshold;
        request->send(200, "text/plain", present ? "present" : "absent");
    });

    server->on("/loxone/device_num", HTTP_GET, [this](AsyncWebServerRequest *request){
        // Numeric: 1 = present, 0 = absent, -1 = unknown
        if (!request->hasParam("address")) {
            request->send(400, "text/plain", "-1");
            return;
        }
        String address = request->getParam("address")->value();
        SafeDevice* devices = deviceManager->getDevices();
        int found = -1;
        for (int i = 0; i < deviceManager->getDeviceCount(); i++) {
            if (address.equalsIgnoreCase(devices[i].address)) {
                found = i;
                break;
            }
        }
        if (found == -1) {
            request->send(200, "text/plain", "-1");
            return;
        }
        bool present = devices[found].isActive && devices[found].isKnown && devices[found].rssi >= devices[found].rssiThreshold;
        request->send(200, "text/plain", present ? "1" : "0");
    });
}

void WebServerManager::handleScanNetworks(AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["status"] = "success";
    
    JsonArray networks = doc["networks"].to<JsonArray>();
    
    // Use synchronous scan (async would require state management)
    // This blocks for ~2-3s but happens rarely (only during setup)
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; i++) {
        JsonObject network = networks.add<JsonObject>();
        network["ssid"] = WiFi.SSID(i);
        network["rssi"] = WiFi.RSSI(i);
        network["auth"] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
    }
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

String WebServerManager::formatRelativeTime(unsigned long seconds) {
    if (seconds < 60) {
        return String(seconds) + "s";
    } else if (seconds < 3600) {
        return String(seconds / 60) + "m " + String(seconds % 60) + "s";
    } else {
        unsigned long hours = seconds / 3600;
        unsigned long minutes = (seconds % 3600) / 60;
        return String(hours) + "h " + String(minutes) + "m";
    }
}

void WebServerManager::updateAllDeviceStatus() {
    // Alle aktiven Geräte durchgehen und Known-Status aktualisieren
    SafeDevice* devices = deviceManager->getDevices();
    int deviceCount = deviceManager->getDeviceCount();
    
    for (int i = 0; i < deviceCount; i++) {
        if (devices[i].isActive) {
            bool wasKnown = devices[i].isKnown;
            bool isNowKnown = deviceManager->isKnownDevice(devices[i].address);
            
            if (wasKnown != isNowKnown) {
                // Status hat sich geändert - Device vollständig aktualisieren
                deviceManager->updateDevice(devices[i].address, devices[i].name, devices[i].rssi);
            }
        }
    }
}

void WebServerManager::handleStatusAPI(AsyncWebServerRequest *request) {
    JsonDocument doc;
    unsigned long uptimeSeconds = millis() / 1000;
    doc["uptime"] = formatRelativeTime(uptimeSeconds);
    doc["devices_ever"] = deviceManager->getTotalEverSeen();  // Geräte total ever
    doc["devices"] = deviceManager->getActiveCount();  // Aktiv (current seen)
    doc["known"] = deviceManager->getKnownCount();  // Bekannt (saved)
    doc["present"] = deviceManager->getPresentCount();  // Anwesend (seen+near)
    doc["wifi_connected"] = wifiManager.isConnected();
    doc["wifi_ssid"] = wifiManager.getSSID();
    doc["wifi_rssi"] = WiFi.isConnected() ? WiFi.RSSI() : 0;
    doc["wifi_ip"] = wifiManager.getLocalIP();
    doc["wifi_gateway"] = WiFi.isConnected() ? WiFi.gatewayIP().toString() : "";
    doc["wifi_subnet"] = WiFi.isConnected() ? WiFi.subnetMask().toString() : "";
    doc["wifi_dns"] = WiFi.isConnected() ? WiFi.dnsIP().toString() : "";
    doc["wifi_mode"] = wifiManager.isConnected() ? "Station" : (wifiManager.isInAPMode() ? "Access Point" : "---");
    doc["heap_free"] = ESP.getFreeHeap();
    
    // Neue Status-Informationen hinzufügen
    doc["scanning"] = bluetoothScanner->isScanning();
    doc["outputActive"] = digitalRead(LED_BUILTIN_PIN) == LOW; // LED AN = LOW wegen invertierter Logik
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void WebServerManager::handleDevicesAPI(AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["status"] = "success";
    
    // Aktive/gefundene Geräte
    JsonArray deviceArray = doc["devices"].to<JsonArray>();
    SafeDevice* devices = deviceManager->getDevices();
    unsigned long now = millis();
    
    for (int i = 0; i < deviceManager->getDeviceCount(); i++) {
        JsonObject device = deviceArray.add<JsonObject>();
        device["address"] = devices[i].address;
        device["name"] = devices[i].name;
        device["rssi"] = devices[i].rssi;
        device["known"] = devices[i].isKnown;
        device["active"] = devices[i].isActive;
        
        // Relative lastSeen Zeit
        if (devices[i].lastSeen > 0) {
            unsigned long ageSeconds = (now - devices[i].lastSeen) / 1000;
            device["lastSeenRelative"] = "vor " + formatRelativeTime(ageSeconds);
        } else {
            device["lastSeenRelative"] = "nie";
        }
        
        device["manufacturer"] = devices[i].manufacturer;
        device["payloadHex"] = devices[i].payloadHex;
        device["comment"] = devices[i].comment;
        device["rssiThreshold"] = devices[i].rssiThreshold;
        
        // Proximity Status: green (nahe genug), yellow (nah aber nicht nah genug), red (nicht sichtbar)
        String proximityStatus = "red";
        if (devices[i].isActive) {
            // Für bekannte Geräte: individueller Schwellwert
            // Für unbekannte Geräte: Standard-Schwellwert
            int threshold = devices[i].isKnown ? devices[i].rssiThreshold : DEFAULT_RSSI_THRESHOLD;
            if (devices[i].rssi >= threshold) {
                proximityStatus = "green"; // Nahe genug
            } else {
                proximityStatus = "yellow"; // Nah aber nicht nah genug
            }
        }
        device["proximityStatus"] = proximityStatus;
    }
    
    // Bekannte Geräte (auch wenn nicht anwesend)
    JsonArray knownArray = doc["knownDevices"].to<JsonArray>();
    char (*knownMACs)[18] = deviceManager->getKnownMACs();
    char (*knownComments)[MAX_COMMENT_LENGTH] = deviceManager->getKnownComments();
    int* knownThresholds = deviceManager->getKnownRSSIThresholds();
    
    for (int i = 0; i < deviceManager->getKnownCount(); i++) {
        JsonObject knownDevice = knownArray.add<JsonObject>();
        knownDevice["address"] = knownMACs[i];
        knownDevice["comment"] = knownComments[i];
        knownDevice["rssiThreshold"] = knownThresholds[i];
        
        // Prüfe ob Gerät aktuell anwesend ist (nur wenn im grünen Proximity-Bereich)
        bool present = false;
        String currentName = "Unbekannt";
        int currentRSSI = -999;
        String proximityStatus = "red";
        
        for (int j = 0; j < deviceManager->getDeviceCount(); j++) {
            if (strcmp(devices[j].address, knownMACs[i]) == 0) {
                currentName = devices[j].name;
                currentRSSI = devices[j].rssi;
                if (devices[j].isActive) {
                    if (devices[j].rssi >= knownThresholds[i]) {
                        proximityStatus = "green";
                        present = true; // ✅ Nur als anwesend gelten wenn nah genug!
                    } else {
                        proximityStatus = "yellow";
                        // present bleibt false - zu weit weg!
                    }
                } else {
                    proximityStatus = "red";
                    // present bleibt false - nicht aktiv
                }
                break;
            }
        }
        
        knownDevice["present"] = present;
        knownDevice["name"] = currentName;
        knownDevice["rssi"] = currentRSSI;
        knownDevice["proximityStatus"] = proximityStatus;
    }
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void WebServerManager::handleWiFiReset(AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["status"] = "success";
    doc["message"] = "WiFi wird zurückgesetzt...";
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
    
    delay(100);
    wifiManager.resetWiFiSettings();
}

void WebServerManager::handleSystemReset(AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["status"] = "success";
    doc["message"] = "System wird neu gestartet...";
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
    
    delay(100);
    ESP.restart();
}

void WebServerManager::handleBluetoothReset(AsyncWebServerRequest *request) {
    JsonDocument doc;
    
    if (bluetoothScanner && bluetoothScanner->isInitialized()) {
        bluetoothScanner->resetBluetooth();
        doc["status"] = "success";
        doc["message"] = "Bluetooth wurde neu gestartet";
    } else {
        doc["status"] = "error";
        doc["message"] = "Bluetooth-Scanner nicht verfügbar";
    }
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void WebServerManager::handleSetKnownDevice(AsyncWebServerRequest *request) {
    if (!request->hasParam("address") || !request->hasParam("known")) {
        sendJSONResponse(request, "error", "Parameter fehlen");
        return;
    }
    
    String address = request->getParam("address")->value();
    bool isKnown = request->getParam("known")->value() == "true";
    String comment = request->hasParam("comment") ? request->getParam("comment")->value() : "";
    int rssiThreshold = request->hasParam("rssiThreshold") ? request->getParam("rssiThreshold")->value().toInt() : -70;
    
    bool success = false;
    if (isKnown) {
        success = deviceManager->addKnownDevice(address.c_str(), comment.c_str(), rssiThreshold) >= 0;
    } else {
        success = deviceManager->removeKnownDevice(address.c_str());
    }
    
    if (success) {
        deviceManager->saveKnownDevices();
        // Sofortige Aktualisierung aller Device-Status
        updateAllDeviceStatus();
        sendJSONResponse(request, "success", isKnown ? "Gerät als bekannt markiert" : "Gerät als unbekannt markiert");
    } else {
        sendJSONResponse(request, "error", "Vorgang fehlgeschlagen");
    }
}

void WebServerManager::handleExportDevicesFile(AsyncWebServerRequest *request) {
    String jsonData = deviceManager->exportDevicesJson();
    
    // Log Export
    char logMsg[80];
    snprintf(logMsg, sizeof(logMsg), "📤 Export: %d Geräte exportiert", deviceManager->getKnownCount());
    deviceManager->logOutputChange("", "", "", false, logMsg);
    
    request->send(200, "application/json", jsonData);
}

void WebServerManager::handleImportDevicesFile(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    // Nutze Request-Objekt um Buffer zu speichern (sicherer als static)
    if (!request->_tempObject) {
        request->_tempObject = new String();
    }
    
    String* bodyBuffer = (String*)request->_tempObject;
    
    // Sammle Daten
    for (size_t i = 0; i < len; i++) {
        *bodyBuffer += (char)data[i];
    }
    
    // Verarbeite nur wenn alle Daten empfangen wurden
    if (index + len == total) {
        bool success = deviceManager->importDevicesJson(*bodyBuffer);
        
        if (success) {
            sendJSONResponse(request, "success", "Geräte erfolgreich importiert");
        } else {
            sendJSONResponse(request, "error", "Import fehlgeschlagen - ungültige Daten");
        }
        
        // Cleanup
        delete bodyBuffer;
        request->_tempObject = nullptr;
    }
}

void WebServerManager::sendJSONResponse(AsyncWebServerRequest *request, const String& status, const String& message) {
    JsonDocument doc;
    doc["status"] = status;
    doc["message"] = message;
    
    String response;
    serializeJson(doc, response);
    request->send(status == "success" ? 200 : 400, "application/json", response);
}

void WebServerManager::handleSetupWiFi(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    static String bodyBuffer = "";
    
    // Sammle alle Body-Daten
    if (index == 0) {
        bodyBuffer = "";
    }
    
    for (size_t i = 0; i < len; i++) {
        bodyBuffer += (char)data[i];
    }
    
    // Verarbeite nur wenn alle Daten empfangen wurden
    if (index + len == total) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, bodyBuffer);
        
        if (error) {
            sendJSONResponse(request, "error", "Ungültige JSON-Daten");
            return;
        }
        
        String ssid = doc["ssid"] | "";
        String password = doc["password"] | "";
        bool scannerMode = doc["scanner"] | false;
        
        // Set scanner mode if requested
        if (scannerMode && modeManager) {
            modeManager->setMode(MODE_SCANNER);
        }
        
        if (ssid.length() == 0) {
            sendJSONResponse(request, "error", "SSID fehlt");
            return;
        }
        
        bool success = false;
        
        // Prüfe ob statische IP konfiguriert werden soll
        if (doc.containsKey("static_ip") && doc["static_ip"].as<String>().length() > 0) {
            String ip = doc["static_ip"] | "";
            String gateway = doc["gateway"] | "";
            String subnet = doc["subnet"] | "255.255.255.0";
            String dns = doc["dns"] | "8.8.8.8";
            
            success = wifiManager.connectToWiFi(ssid, password, ip, gateway, subnet, dns);
        } else {
            // Normale Verbindung ohne statische IP
            success = wifiManager.connectToWiFi(ssid, password);
        }
        
        if (success) {
            sendJSONResponse(request, "success", "WiFi verbunden! IP: " + wifiManager.getLocalIP());
            // Nach erfolgreicher Verbindung Haupt-Routen aktivieren
            this->end();
            this->begin(deviceManager, bluetoothScanner);
        } else {
            sendJSONResponse(request, "error", "WiFi Verbindung fehlgeschlagen");
        }
    }
}

void WebServerManager::handleSetupAP(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    static String bodyBuffer = "";
    
    // Sammle alle Body-Daten
    if (index == 0) {
        bodyBuffer = "";
    }
    
    for (size_t i = 0; i < len; i++) {
        bodyBuffer += (char)data[i];
    }
    
    // Verarbeite nur wenn alle Daten empfangen wurden
    if (index + len == total) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, bodyBuffer);
        
        if (error) {
            sendJSONResponse(request, "error", "Ungültige JSON-Daten");
            return;
        }
        
        String password = doc["password"] | "";
        bool scannerMode = doc["scanner"] | false;
        
        // Set scanner mode if requested
        if (scannerMode && modeManager) {
            modeManager->setMode(MODE_SCANNER);
        }
        
        if (password.length() < 8) {
            sendJSONResponse(request, "error", "Passwort muss mindestens 8 Zeichen haben");
            return;
        }
        
        if (wifiManager.setAPPassword(password)) {
            sendJSONResponse(request, "success", "AP-Passwort gesetzt, System startet neu...");
            delay(100);
            ESP.restart();
        } else {
            sendJSONResponse(request, "error", "Fehler beim Setzen des AP-Passworts");
        }
    }
}

// ================== Mode Setup Methods ==================

void WebServerManager::startSetupMode() {
    setupComplete = false;
    
    // Stop existing server if running
    if (server) {
        server->end();
        delete server;
        server = nullptr;
    }
    
    // Create fresh server for mode setup
    server = new AsyncWebServer(80);
    
    // Start DNS server for captive portal (redirect all DNS requests to our AP IP)
    if (!dnsServer) {
        dnsServer = new DNSServer();
    }
    dnsServer->start(53, "*", WiFi.softAPIP());
    
    // Main setup page - unified mode selection
    server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
        handleModeSetup(request);
    });
    
    // Network scan API
    server->on("/api/scan", HTTP_GET, [this](AsyncWebServerRequest *request){
        handleScanNetworks(request);
    });
    
    // Beacon configuration via GET with query parameters (simpler for JavaScript fetch)
    server->on("/setup/beacon", HTTP_POST, [this](AsyncWebServerRequest *request){
        String name = request->hasParam("name") ? request->getParam("name")->value() : "";
        int interval = request->hasParam("interval") ? request->getParam("interval")->value().toInt() : 800;
        int power = request->hasParam("power") ? request->getParam("power")->value().toInt() : 0;
        
        // If name is empty or just whitespace, use default "BT-beacon" (triggers MAC suffix)
        name.trim();
        if (name.length() == 0) {
            name = "BT-beacon";
        }
        
        // Save config (will auto-erase NVS if name changed)
        if (modeManager) {
            modeManager->setMode(MODE_BEACON);
            modeManager->setBeaconConfig(name.c_str(), interval, power);
            setupComplete = true;
        }
        
        request->send(200, "text/plain", "OK");
        delay(100);
        ESP.restart();
    });
    
    // WiFi setup with scanner mode flag
    server->on("/setup/wifi", HTTP_POST, [this](AsyncWebServerRequest *request){
        sendJSONResponse(request, "success", "WLAN wird verbunden...");
    }, nullptr, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
        handleSetupWiFi(request, data, len, index, total);
    });
    
    // AP setup with scanner mode flag
    server->on("/setup/ap", HTTP_POST, [this](AsyncWebServerRequest *request){
        sendJSONResponse(request, "success", "AP wird eingerichtet...");
    }, nullptr, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
        handleSetupAP(request, data, len, index, total);
    });
    
    // Catch-all for captive portal - redirect all requests to main page
    server->onNotFound([this](AsyncWebServerRequest *request){
        handleModeSetup(request);
    });
    
    server->begin();
    isRunning = true;
}

void WebServerManager::handleModeSetup(AsyncWebServerRequest *request) {
    // Get current beacon configuration
    BeaconConfig currentConfig = modeManager->getBeaconConfig();
    String currentName = modeManager->getBeaconName();
    
    // Use modular WebUI to generate HTML
    String html = WebUI::generateModeSetupHTML(currentName, currentConfig);
    
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", html);
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "0");
    request->send(response);
}

void WebServerManager::handleBeaconConfigPage(AsyncWebServerRequest *request) {
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>Beacon Setup</title>";
    html += "<style>body{font-family:Arial;margin:40px;} input,select{margin:10px 0;padding:8px;width:200px;} button{padding:15px 30px;background:#0066cc;color:white;border:none;border-radius:5px;cursor:pointer;margin-top:20px;}</style>";
    html += "</head><body>";
    html += "<h1>Beacon-Modus</h1>";
    html += "<form method='POST' action='/setup/beacon'>";
    html += "<label>Name:<br><input type='text' name='name' value='BT-beacon'></label><br>";
    html += "<label>Intervall (ms):<br><input type='number' name='interval' value='800' min='100' max='10000'></label><br>";
    html += "<label>TX Power:<br><select name='power'>";
    html += "<option value='-12'>-12 dBm (2m)</option>";
    html += "<option value='-9'>-9 dBm (3m)</option>";
    html += "<option value='-6'>-6 dBm (5m)</option>";
    html += "<option value='-3'>-3 dBm (7m)</option>";
    html += "<option value='0' selected>0 dBm (10m)</option>";
    html += "<option value='3'>3 dBm (15m)</option>";
    html += "<option value='6'>6 dBm (20m)</option>";
    html += "<option value='9'>9 dBm (30m)</option>";
    html += "</select></label><br>";
    html += "<button type='submit'>Speichern und Starten</button>";
    html += "</form>";
    html += "<p><a href='/'>Zurueck</a></p>";
    html += "</body></html>";
    
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", html);
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "0");
    request->send(response);
}

void WebServerManager::handleScannerConfigPage(AsyncWebServerRequest *request) {
    if (modeManager) {
        modeManager->setMode(MODE_SCANNER);
        setupComplete = true;
    }
    
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta http-equiv='refresh' content='3;url=/'>";
    html += "<title>Scanner Setup</title>";
    html += "<style>body{font-family:Arial;margin:40px;text-align:center;}</style>";
    html += "</head><body>";
    html += "<h1>Scanner-Modus aktiviert!</h1>";
    html += "<p>Geraet startet neu...</p>";
    html += "<p>Danach WiFi-Setup verfuegbar.</p>";
    html += "</body></html>";
    
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", html);
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    request->send(response);
    
    // Restart nach kurzer Verzoegerung
    delay(100);
    ESP.restart();
}

void WebServerManager::processDNS() {
    if (dnsServer) {
        dnsServer->processNextRequest();
    }
}

void WebServerManager::handleModeSelection(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    // Not used with simple HTML
}

void WebServerManager::handleBeaconConfig(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    // Not used with simple HTML
}
