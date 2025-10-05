#include "WebServerManager.h"
#include "Config.h"
#include "WiFiManager.h"

extern WiFiManager wifiManager;

WebServerManager::WebServerManager() : server(nullptr), setupServer(nullptr), deviceManager(nullptr), bluetoothScanner(nullptr), isRunning(false), setupServerStarted(false), isInSecureMode(false) {}

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

void WebServerManager::setupMainServerRoutes() {
    if (!server) return;
    
    server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
        request->send(200, "text/html", generateMainHTML());
    });
    
    server->on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request){
        handleStatusAPI(request);
    });
    
    server->on("/api/devices", HTTP_GET, [this](AsyncWebServerRequest *request){
        handleDevicesAPI(request);
    });
    
    server->on("/api/wifi/reset", HTTP_POST, [this](AsyncWebServerRequest *request){
        handleWiFiReset(request);
    });
    
    server->on("/api/system/reset", HTTP_POST, [this](AsyncWebServerRequest *request){
        handleSystemReset(request);
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
    
    server->on("/api/import-devices-file", HTTP_POST, [this](AsyncWebServerRequest *request){
        // Empty handler - body processing happens in body handler
    }, nullptr, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
        handleImportDevicesFile(request, data, len, index, total);
    });
    
    // Output Log APIs
    server->on("/api/output-log", HTTP_GET, [this](AsyncWebServerRequest *request){
        if (!deviceManager) {
            request->send(500, "application/json", "{\"status\":\"error\",\"message\":\"DeviceManager not initialized\"}");
            return;
        }
        
        String logJson = deviceManager->getOutputLogJson();
        request->send(200, "application/json", logJson);
    });
    
    server->on("/api/output-log/clear", HTTP_POST, [this](AsyncWebServerRequest *request){
        if (!deviceManager) {
            request->send(500, "application/json", "{\"status\":\"error\",\"message\":\"DeviceManager not initialized\"}");
            return;
        }
        
        deviceManager->clearOutputLog();
        request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Output-Log gelöscht\"}");
    });
    
    // TEST-Route für manuellen Log-Eintrag
    server->on("/api/output-log/test", HTTP_POST, [this](AsyncWebServerRequest *request){
        if (!deviceManager) {
            request->send(500, "application/json", "{\"status\":\"error\",\"message\":\"DeviceManager not initialized\"}");
            return;
        }
        
        // Erstelle Test-Log-Eintrag
        deviceManager->logOutputChange("00:11:22:33:44:55", "Test-Gerät", "Manueller Test", true, "Test-Eintrag erstellt");
        request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Test-Log-Eintrag erstellt (LED sollte 5x blinken)\"}");
    });
    
    setupAPIRoutes();
    setupLoxoneRoutes();
}

void WebServerManager::setupSetupRoutes() {
    if (!server) return;
    
    server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
        handleSetupRoot(request);
    });
    
    server->on("/setup", HTTP_GET, [this](AsyncWebServerRequest *request){
        handleSetupRoot(request);
    });
    
    server->on("/setup/wifi", HTTP_POST,
        [this](AsyncWebServerRequest *request){
            // Empty handler - processing happens in body handler
        },
        nullptr,
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
            handleSetupWiFi(request, data, len, index, total);
        }
    );
    
    server->on("/setup/ap", HTTP_POST,
        [this](AsyncWebServerRequest *request){
            // Empty handler - processing happens in body handler  
        },
        nullptr,
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
            handleSetupAP(request, data, len, index, total);
        }
    );
    
    server->on("/api/scan", HTTP_GET, [this](AsyncWebServerRequest *request){
        handleScanNetworks(request);
    });
}

void WebServerManager::handleSetupRoot(AsyncWebServerRequest *request) {
    request->send(200, "text/html", generateSetupHTML());
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
        if (!error) {
            String ssid = doc["ssid"].as<String>();
            String password = doc["password"].as<String>();
            
            if (ssid.length() > 0) {
                // Sende Antwort zuerst
                JsonDocument responseDoc;
                responseDoc["status"] = "success";
                responseDoc["message"] = "WLAN-Verbindung wird aufgebaut...";
                
                String response;
                serializeJson(responseDoc, response);
                request->send(200, "application/json", response);
                
                // Dann WiFi konfigurieren (delayed)
                delay(100);
                wifiManager.connectToWiFi(ssid, password);
                
                return;
            }
        }
        
        // Fehler-Antwort
        JsonDocument errorDoc;
        errorDoc["status"] = "error";
        errorDoc["message"] = "Ungültige JSON-Daten oder fehlende SSID";
        
        String errorResponse;
        serializeJson(errorDoc, errorResponse);
        request->send(400, "application/json", errorResponse);
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
        if (!error) {
            String password = doc["password"].as<String>();
            
            if (password.length() >= 8) {
                // Sende Antwort zuerst
                JsonDocument responseDoc;
                responseDoc["status"] = "success";
                responseDoc["message"] = "Sicherer AP wird eingerichtet...";
                
                String response;
                serializeJson(responseDoc, response);
                request->send(200, "application/json", response);
                
                // Dann AP konfigurieren (delayed)
                delay(100);
                wifiManager.startAccessPoint(password);
                
                return;
            }
        }
        
        // Fehler-Antwort
        JsonDocument errorDoc;
        errorDoc["status"] = "error";
        errorDoc["message"] = "Passwort muss mindestens 8 Zeichen haben";
        
        String errorResponse;
        serializeJson(errorDoc, errorResponse);
        request->send(400, "application/json", errorResponse);
    }
}

void WebServerManager::handleScanNetworks(AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["status"] = "success";
    
    JsonArray networks = doc["networks"].to<JsonArray>();
    
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
    doc["devices"] = deviceManager->getDeviceCount();
    doc["known"] = deviceManager->getKnownCount();
    doc["wifi_connected"] = wifiManager.isConnected();
    doc["wifi_ssid"] = wifiManager.getSSID();
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
    char (*knownComments)[50] = deviceManager->getKnownComments();
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
    
    delay(500);
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
    
    request->send(200, "application/json", jsonData);
}

void WebServerManager::handleImportDevicesFile(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
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
        bool success = deviceManager->importDevicesJson(bodyBuffer);
        
        if (success) {
            sendJSONResponse(request, "success", "Geräte erfolgreich importiert");
        } else {
            sendJSONResponse(request, "error", "Import fehlgeschlagen - ungültige Daten");
        }
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

void WebServerManager::setupAPIRoutes() {
    // Weitere API-Routen falls benötigt
}

void WebServerManager::setupLoxoneRoutes() {
    // Loxone-spezifische Routen falls benötigt
}



String WebServerManager::generateSetupHTML() {
    return String(
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset=\"UTF-8\">"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
        "<title>ESP32-C3 Setup</title>"
        "<style>"
        "body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }"
        ".container { max-width: 500px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }"
        "h1 { text-align: center; color: #333; margin-bottom: 30px; }"
        ".form-group { margin-bottom: 20px; }"
        "label { display: block; margin-bottom: 5px; font-weight: bold; }"
        "input, select { width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 5px; box-sizing: border-box; }"
        "button { width: 100%; padding: 12px; background: #007bff; color: white; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; }"
        "button:hover { background: #0056b3; }"
        ".tab { display: none; }"
        ".tab.active { display: block; }"
        ".tab-buttons { display: flex; margin-bottom: 20px; }"
        ".tab-button { flex: 1; padding: 10px; background: #e9ecef; border: none; cursor: pointer; }"
        ".tab-button.active { background: #007bff; color: white; }"
        ".networks { max-height: 200px; overflow-y: auto; border: 1px solid #ddd; border-radius: 5px; }"
        ".network { padding: 10px; border-bottom: 1px solid #eee; cursor: pointer; }"
        ".network:hover { background: #f8f9fa; }"
        ".network:last-child { border-bottom: none; }"
        "#message { margin-top: 20px; padding: 10px; border-radius: 5px; display: none; }"
        ".success { background: #d4edda; color: #155724; border: 1px solid #c3e6cb; }"
        ".error { background: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }"
        "</style>"
        "</head>"
        "<body>"
        "<div class=\"container\">"
        "<h1>🔧 ESP32-C3 Setup</h1>"
        
        "<div class=\"tab-buttons\">"
        "<button class=\"tab-button active\" onclick=\"showTab('wifi')\">WLAN verbinden</button>"
        "<button class=\"tab-button\" onclick=\"showTab('ap')\">Sicherer AP</button>"
        "</div>"
        
        "<div id=\"wifi-tab\" class=\"tab active\">"
        "<div class=\"form-group\">"
        "<label>Verfügbare Netzwerke:</label>"
        "<div id=\"networks\" class=\"networks\">Lade Netzwerke...</div>"
        "</div>"
        "<div class=\"form-group\">"
        "<label for=\"wifi-ssid\">SSID:</label>"
        "<input type=\"text\" id=\"wifi-ssid\" placeholder=\"Netzwerkname\">"
        "</div>"
        "<div class=\"form-group\">"
        "<label for=\"wifi-password\">Passwort:</label>"
        "<input type=\"password\" id=\"wifi-password\" placeholder=\"WLAN-Passwort\">"
        "</div>"
        "<button onclick=\"connectWiFi()\">Verbinden</button>"
        "</div>"
        
        "<div id=\"ap-tab\" class=\"tab\">"
        "<p>Erstelle einen sicheren Access Point für den ESP32.</p>"
        "<div class=\"form-group\">"
        "<label for=\"ap-password\">AP-Passwort:</label>"
        "<input type=\"password\" id=\"ap-password\" placeholder=\"Mindestens 8 Zeichen\">"
        "</div>"
        "<button onclick=\"setupAP()\">AP erstellen</button>"
        "</div>"
        
        "<div id=\"message\"></div>"
        "</div>"
        
        "<script>"
        "function showTab(tab) {"
        "document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));"
        "document.querySelectorAll('.tab-button').forEach(b => b.classList.remove('active'));"
        "document.getElementById(tab + '-tab').classList.add('active');"
        "event.target.classList.add('active');"
        "}"
        
        "function showMessage(text, isError = false) {"
        "const msg = document.getElementById('message');"
        "msg.textContent = text;"
        "msg.className = isError ? 'error' : 'success';"
        "msg.style.display = 'block';"
        "setTimeout(() => msg.style.display = 'none', 5000);"
        "}"
        
        "function connectWiFi() {"
        "const ssid = document.getElementById('wifi-ssid').value;"
        "const password = document.getElementById('wifi-password').value;"
        "if (!ssid) { showMessage('Bitte SSID eingeben', true); return; }"
        "fetch('/setup/wifi', {"
        "method: 'POST',"
        "headers: { 'Content-Type': 'application/json' },"
        "body: JSON.stringify({ ssid: ssid, password: password })"
        "})"
        ".then(response => response.json())"
        ".then(data => {"
        "showMessage(data.message, data.status !== 'success');"
        "if (data.status === 'success') {"
        "setTimeout(() => {"
        "showMessage('Seite wird neu geladen...');"
        "setTimeout(() => window.location.reload(), 2000);"
        "}, 3000);"
        "}"
        "})"
        ".catch(error => {"
        "setTimeout(() => {"
        "showMessage('Verbindung wird aufgebaut, Seite wird neu geladen...');"
        "setTimeout(() => window.location.reload(), 3000);"
        "}, 2000);"
        "});"
        "}"
        
        "function setupAP() {"
        "const password = document.getElementById('ap-password').value;"
        "if (password.length < 8) { showMessage('Passwort muss mindestens 8 Zeichen haben', true); return; }"
        "fetch('/setup/ap', {"
        "method: 'POST',"
        "headers: { 'Content-Type': 'application/json' },"
        "body: JSON.stringify({ password: password })"
        "})"
        ".then(response => response.json())"
        ".then(data => {"
        "showMessage(data.message, data.status !== 'success');"
        "if (data.status === 'success') {"
        "setTimeout(() => window.location.reload(), 5000);"
        "}"
        "})"
        ".catch(error => {"
        "setTimeout(() => {"
        "showMessage('AP wird eingerichtet, Seite wird neu geladen...');"
        "setTimeout(() => window.location.reload(), 5000);"
        "}, 2000);"
        "});"
        "}"
        
        "function loadNetworks() {"
        "fetch('/api/scan')"
        ".then(response => response.json())"
        ".then(data => {"
        "const container = document.getElementById('networks');"
        "container.innerHTML = '';"
        "data.networks.forEach(network => {"
        "const div = document.createElement('div');"
        "div.className = 'network';"
        "div.innerHTML = `<strong>${network.ssid}</strong> (${network.rssi} dBm) ${network.auth ? '🔒' : '🔓'}`;"
        "div.onclick = () => document.getElementById('wifi-ssid').value = network.ssid;"
        "container.appendChild(div);"
        "});"
        "})"
        ".catch(error => {"
        "document.getElementById('networks').innerHTML = 'Fehler beim Laden der Netzwerke';"
        "});"
        "}"
        
        "loadNetworks();"
        "</script>"
        "</body>"
        "</html>"
    );
}

String WebServerManager::generateMainHTML() {
    return String(
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset=\"UTF-8\">"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
        "<title>🔍 ESP32-C3 BT Scanner</title>"
        "<style>"
        "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif; margin: 0; padding: 20px; background: #f5f5f7; color: #1d1d1f; }"
        ".container { max-width: 1200px; margin: 0 auto; }"
        ".header { background: white; padding: 20px; border-radius: 12px; box-shadow: 0 4px 12px rgba(0,0,0,0.1); margin-bottom: 20px; }"
        ".header h1 { margin: 0 0 10px 0; color: #1d1d1f; font-size: 2em; }"
        ".status-bar { display: flex; gap: 20px; flex-wrap: wrap; margin-top: 15px; }"
        ".status-item { background: #f6f6f6; padding: 10px 15px; border-radius: 8px; font-size: 14px; }"
        ".status-item.online { background: #d4edda; color: #155724; }"
        ".controls { background: white; padding: 20px; border-radius: 12px; box-shadow: 0 4px 12px rgba(0,0,0,0.1); margin-bottom: 20px; }"
        ".btn-group { display: flex; gap: 10px; flex-wrap: wrap; }"
        "button { background: #007aff; color: white; border: none; padding: 12px 20px; border-radius: 8px; cursor: pointer; font-size: 14px; font-weight: 500; transition: all 0.2s; }"
        "button:hover { background: #0056d3; transform: translateY(-1px); }"
        "button.secondary { background: #8e8e93; }"
        "button.secondary:hover { background: #6d6d70; }"
        "button.danger { background: #ff3b30; }"
        "button.danger:hover { background: #d70015; }"
        "button.success { background: #34c759; }"
        "button.success:hover { background: #248a3d; }"
        ".devices-section { background: white; border-radius: 12px; box-shadow: 0 4px 12px rgba(0,0,0,0.1); overflow: hidden; }"
        ".devices-header { padding: 20px; border-bottom: 1px solid #e5e5e7; display: flex; justify-content: space-between; align-items: center; }"
        ".devices-header h2 { margin: 0; font-size: 1.5em; }"
        ".filter-controls { display: flex; gap: 10px; align-items: center; }"
        ".filter-controls select, .filter-controls input { padding: 8px 12px; border: 1px solid #d1d1d6; border-radius: 6px; }"
        ".device { padding: 15px 20px; border-bottom: 1px solid #f2f2f7; transition: background 0.2s; }"
        ".device:hover { background: #f9f9f9; }"
        ".device:last-child { border-bottom: none; }"
        ".device.known { background: #f0f8ff; border-left: 4px solid #007aff; }" // Bekannt = blaue Markierung
        ".device.active { background: #f9f9f9; }" // Aktiv = nur leicht grau
        ".device.proximity-green { border-left: 4px solid #34c759; background: #e8f5e8 !important; }" // Grün überschreibt alles
        ".device.proximity-yellow { border-left: 4px solid #ff9500; background: #fff8e8 !important; }" // Gelb überschreibt alles
        ".device.proximity-red { border-left: 4px solid #ff3b30; background: #ffeaea !important; }" // Rot überschreibt alles
        ".device-row { display: flex; justify-content: space-between; align-items: center; }"
        ".device-info { flex: 1; }"
        ".device-name { font-weight: 600; font-size: 16px; margin-bottom: 4px; }"
        ".device-details { display: flex; gap: 15px; font-size: 14px; color: #8e8e93; margin-bottom: 4px; }"
        ".device-meta { font-size: 12px; color: #a1a1a6; }"
        ".device-actions { display: flex; gap: 8px; }"
        ".device-actions button { padding: 6px 12px; font-size: 12px; }"
        ".stats { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px; margin-bottom: 20px; }"
        ".stat-card { background: white; padding: 20px; border-radius: 12px; box-shadow: 0 2px 8px rgba(0,0,0,0.1); text-align: center; }"
        ".stat-number { font-size: 2em; font-weight: 700; color: #007aff; margin-bottom: 5px; }"
        ".stat-label { color: #8e8e93; font-size: 14px; }"
        ".notification { position: fixed; top: 20px; right: 20px; padding: 15px 20px; background: #34c759; color: white; border-radius: 8px; box-shadow: 0 4px 12px rgba(0,0,0,0.2); z-index: 1001; transform: translateX(400px); transition: transform 0.3s; }"
        ".notification.show { transform: translateX(0); }"
        ".notification.error { background: #ff3b30; }"
        ".file-input { display: none; }"
        ".file-label { background: #007aff; color: white; border: none; padding: 12px 20px; border-radius: 8px; cursor: pointer; font-size: 14px; font-weight: 500; text-align: center; transition: all 0.2s; display: inline-block; }"
        ".file-label:hover { background: #0056d3; transform: translateY(-1px); }"
        "@media (max-width: 768px) {"
        ".status-bar { flex-direction: column; }"
        ".btn-group { flex-direction: column; }"
        ".device-row { flex-direction: column; align-items: flex-start; }"
        ".device-actions { margin-top: 10px; }"
        "}"
        "</style>"
        "</head>"
        "<body>"
        "<div class=\"container\">"
        "<div class=\"header\">"
        "<h1>🔍 ESP32-C3 BT Scanner</h1>"
        "<div class=\"status-bar\">"
        "<div id=\"status-wifi\" class=\"status-item\">WiFi: Verbinde...</div>"
        "<div id=\"status-devices\" class=\"status-item\">📱 Geräte: <span id=\"stat-total\">0</span></div>"
        "<div id=\"status-known\" class=\"status-item\">⭐ Bekannt: <span id=\"stat-known\">0</span></div>"
        "<div id=\"status-active\" class=\"status-item\">🟢 Aktiv: <span id=\"stat-active\">0</span></div>"
        "<div id=\"status-present\" class=\"status-item\">✅ Anwesend: <span id=\"stat-present\">0</span></div>"
        "<div id=\"status-scan\" class=\"status-item\">🔍 <span id=\"scan-status\">Scan: Lade...</span></div>"
        "<div id=\"status-output\" class=\"status-item\">🔗 <span id=\"output-status\">Ausgang: Lade...</span></div>"
        "<div id=\"status-uptime\" class=\"status-item\">Uptime: Lade...</div>"
        "</div>"
        "</div>"
        
        "<div class=\"controls\">"
        "<h3>Steuerung</h3>"
        "<div class=\"btn-group\">"
        "<button onclick=\"refreshData()\" class=\"success\">🔄 Aktualisieren</button>"
        "<button onclick=\"exportDevices()\">📥 Export</button>"
        "<label for=\"import-file\" class=\"file-label\">📤 Import</label>"
        "<input type=\"file\" id=\"import-file\" class=\"file-input\" accept=\".json\" onchange=\"importDevices(this)\">"
        "<button onclick=\"resetWiFi()\" class=\"danger\">📶 WiFi Reset</button>"
        "<button onclick=\"resetBluetooth()\" class=\"danger\">🔵 Bluetooth Reset</button>"
        "<button onclick=\"resetSystem()\" class=\"danger\">🔄 System Reboot</button>"
        "</div>"
        "</div>"
        
        "<div class=\"devices-section\">"
        "<div class=\"devices-header\">"
        "<h2>Aktuell gefundene Geräte</h2>"
        "<div class=\"filter-controls\">"
        "<small>Nur aktive Geräte in Bluetooth-Reichweite</small>"
        "</div>"
        "</div>"
        "<div id=\"devices-list\"></div>"
        "</div>"
        
        "<div class=\"devices-section\">"
        "<div class=\"devices-header\">"
        "<h2>Bekannte Geräte</h2>"
        "<div class=\"filter-controls\">"
        "<small>Alle bekannten Geräte (auch wenn nicht anwesend)</small>"
        "</div>"
        "</div>"
        "<div id=\"known-devices-list\"></div>"
        "</div>"
        
        "<div class=\"devices-section\">"
        "<div class=\"devices-header\">"
        "<h2>Ausgangs-Log</h2>"
        "<div class=\"filter-controls\">"
        "<button id=\"clear-log-btn\" class=\"action-btn\">Log löschen</button>"
        "</div>"
        "</div>"
        "<div id=\"output-log-list\"></div>"
        "</div>"
        "</div>"
        
        "<div id=\"notification\" class=\"notification\"></div>"
    ) + generateMainHTMLScript();
}

String WebServerManager::generateMainHTMLScript() {
    return String(
        "<script>"
        "let devices = [];"
        "let filteredDevices = [];"
        "let knownDevices = [];"
        "function showNotification(message, isError) {"
        "const notification = document.getElementById('notification');"
        "notification.textContent = message;"
        "notification.className = 'notification' + (isError ? ' error' : '') + ' show';"
        "setTimeout(() => { notification.className = 'notification'; }, 4000);"
        "}"
        "function loadDevices() {"
        "fetch('/api/devices').then(response => response.json()).then(data => {"
        "devices = data.devices || [];"
        "knownDevices = data.knownDevices || [];"
        "updateStats();"
        "filterDevices();"
        "renderKnownDevices();"
        "}).catch(error => {"
        "console.error('Geräte-Fehler:', error);"
        "showNotification('Fehler beim Laden der Geräte', true);"
        "});"
        "fetch('/api/status').then(response => response.json()).then(data => {"
        "document.getElementById('scan-status').textContent = data.scanning ? 'Scannt...' : 'Pause';"
        "document.getElementById('output-status').textContent = data.outputActive ? 'AN' : 'AUS';"
        "document.getElementById('status-scan').className = 'status-item' + (data.scanning ? ' online' : '');"
        "document.getElementById('status-output').className = 'status-item' + (data.outputActive ? ' online' : '');"
        "const wifiStatus = data.wifi_connected ? 'WiFi: ' + data.wifi_ssid : 'WiFi: Nicht verbunden';"
        "document.getElementById('status-wifi').textContent = wifiStatus;"
        "document.getElementById('status-wifi').className = 'status-item' + (data.wifi_connected ? ' online' : '');"
        "document.getElementById('status-uptime').textContent = 'Uptime: ' + data.uptime;"
        "}).catch(error => console.error('Status-Fehler:', error));"
        "loadOutputLog();"
        "}"
        "function loadOutputLog() {"
        "fetch('/api/output-log').then(response => response.json()).then(data => {"
        "const logEntries = data.outputLog || [];"
        "renderOutputLog(logEntries);"
        "}).catch(error => {"
        "console.error('Output-Log Fehler:', error);"
        "});"
        "}"
        "function renderOutputLog(logEntries) {"
        "const container = document.getElementById('output-log-list');"
        "if (logEntries.length === 0) {"
        "container.innerHTML = '<div style=\"padding: 40px; text-align: center; color: #8e8e93;\">Keine Log-Einträge</div>';"
        "return;"
        "}"
        "container.innerHTML = logEntries.map(entry => {"
        "const stateIcon = entry.outputState ? '🟢 AN' : '🔴 AUS';"
        "const deviceInfo = entry.deviceName && entry.deviceName !== '' ? entry.deviceName + ' (' + entry.deviceAddress + ')' : entry.deviceAddress;"
        "const timeInfo = entry.timeAgo ? 'vor ' + entry.timeAgo : 'gerade eben';"
        "return '<div class=\"device\"><div class=\"device-row\"><div class=\"device-info\"><div class=\"device-name\">' + timeInfo + ' - ' + stateIcon + '</div><div class=\"device-details\"><span>📱 ' + deviceInfo + '</span><span>📝 ' + entry.reason + '</span></div></div></div></div>';"
        "}).join('');"
        "}"
        "function clearOutputLog() {"
        "if (!confirm('Log wirklich löschen?')) return;"
        "fetch('/api/output-log/clear', { method: 'POST' }).then(response => response.json()).then(data => {"
        "if (data.status === 'success') {"
        "showNotification('Log gelöscht');"
        "loadOutputLog();"
        "} else {"
        "showNotification('Fehler beim Löschen', true);"
        "}"
        "}).catch(error => {"
        "showNotification('Fehler beim Löschen', true);"
        "});"
        "}"
        "function updateStats() {"
        "const total = devices.length;"
        "const known = devices.filter(d => d.known).length;"
        "const active = devices.filter(d => d.active).length;"
        "const present = devices.filter(d => d.proximityStatus === 'green').length;"
        "document.getElementById('stat-total').textContent = total;"
        "document.getElementById('stat-known').textContent = known;"
        "document.getElementById('stat-active').textContent = active;"
        "document.getElementById('stat-present').textContent = present;"
        "}"
        "function filterDevices() {"
        "filteredDevices = devices.filter(device => device.active);"
        "renderDevices();"
        "}"
        "function renderDevices() {"
        "const container = document.getElementById('devices-list');"
        "if (filteredDevices.length === 0) {"
        "container.innerHTML = '<div style=\"padding: 40px; text-align: center; color: #8e8e93;\">Keine Geräte gefunden</div>';"
        "return;"
        "}"
        "container.innerHTML = filteredDevices.map(device => {"
        "const lastSeen = device.lastSeenRelative || 'nie';"
        "const manufacturerInfo = device.manufacturer || 'Unbekannt';"
        "const comment = device.comment || '';"
        "const proximityClass = ' proximity-' + (device.proximityStatus || 'red');"
        "return '<div class=\"device' + (device.known ? ' known' : '') + (device.active ? ' active' : '') + proximityClass + '\"><div class=\"device-row\"><div class=\"device-info\"><div class=\"device-name\">' + (device.name || 'Unbekanntes Gerät') + (comment ? ' (' + comment + ')' : '') + '</div><div class=\"device-details\"><span>📱 ' + device.address + '</span><span>📶 ' + device.rssi + ' dBm</span><span title=\"Payload: ' + (device.payloadHex ? device.payloadHex : 'Keine Daten') + '\">🏭 ' + (manufacturerInfo !== 'Unbekannt' ? manufacturerInfo : (device.payloadHex ? 'Raw: ' + device.payloadHex.substring(0, 8) + '...' : 'Unbekannt')) + '</span>' + (device.known ? '<span>⭐ Bekannt (Schwelle: ' + (device.rssiThreshold || -80) + ' dBm)</span>' : '') + getProximityStatusText(device.proximityStatus) + '</div><div class=\"device-meta\">Zuletzt gesehen: ' + lastSeen + '</div></div><div class=\"device-actions\"><button onclick=\"toggleKnown(\\'' + device.address + '\\', ' + !device.known + ')\" class=\"' + (device.known ? 'danger' : 'success') + '\">' + (device.known ? '❌ Entfernen' : '⭐ Hinzufügen') + '</button></div></div></div>';"
        "}).join('');"
        "}"
        "function getProximityStatusText(status) {"
        "switch(status) {"
        "case 'green': return '<span style=\"color: #34c759;\">🟢 In Reichweite</span>';"
        "case 'yellow': return '<span style=\"color: #ff9500;\">🟡 Nah</span>';"
        "case 'red': return '<span style=\"color: #ff3b30;\">🔴 Nicht sichtbar</span>';"
        "default: return '<span style=\"color: #8e8e93;\">❓ Unbekannt</span>';"
        "}"
        "}"
        "function renderKnownDevices() {"
        "const container = document.getElementById('known-devices-list');"
        "if (knownDevices.length === 0) {"
        "container.innerHTML = '<div style=\"padding: 40px; text-align: center; color: #8e8e93;\">Keine bekannten Geräte</div>';"
        "return;"
        "}"
        "container.innerHTML = knownDevices.map(device => {"
        "const proximityClass = ' proximity-' + (device.proximityStatus || 'red');"
        "return '<div class=\"device known' + proximityClass + '\"><div class=\"device-row\"><div class=\"device-info\"><div class=\"device-name\">' + (device.name || 'Unbekanntes Gerät') + ' (' + (device.comment || 'Kein Kommentar') + ')</div><div class=\"device-details\"><span>📱 ' + device.address + '</span>' + (device.rssi ? '<span>📶 ' + device.rssi + ' dBm</span>' : '<span>📶 --- dBm</span>') + '<span>⚡ Schwelle: ' + device.rssiThreshold + ' dBm</span>' + getProximityStatusText(device.proximityStatus) + '</div></div><div class=\"device-actions\"><button onclick=\"toggleKnown(\\'' + device.address + '\\', false)\" class=\"danger\">❌ Entfernen</button></div></div></div>';"
        "}).join('');"
        "}"
        "function toggleKnown(address, known) {"
        "if (known) {"
        "showDeviceDialog(address);"
        "} else {"
        "const params = new URLSearchParams({ address: address, known: known });"
        "fetch('/api/device/known?' + params, { method: 'POST' }).then(response => response.json()).then(data => {"
        "if (data.status === 'success') {"
        "showNotification(data.message);"
        "loadDevices();"
        "} else {"
        "showNotification(data.message, true);"
        "}"
        "}).catch(error => {"
        "showNotification('Fehler beim Ändern des Status', true);"
        "});"
        "}"
        "}"
        "function showDeviceDialog(address) {"
        "const dialog = document.createElement('div');"
        "dialog.style.cssText = 'position:fixed;top:0;left:0;width:100%;height:100%;background:rgba(0,0,0,0.5);z-index:1000;display:flex;align-items:center;justify-content:center';"
        "dialog.innerHTML = '<div style=\"background:white;padding:20px;border-radius:12px;width:90%;max-width:400px;box-shadow:0 10px 30px rgba(0,0,0,0.3)\"><h3 style=\"margin:0 0 15px\">Gerät hinzufügen</h3><p style=\"color:#666;margin-bottom:15px\">MAC: ' + address + '</p><input type=\"text\" id=\"device-comment\" placeholder=\"Kommentar (z.B. iPhone von Max)\" style=\"width:100%;padding:10px;border:1px solid #ddd;border-radius:6px;margin-bottom:10px\"><input type=\"number\" id=\"device-threshold\" value=\"-80\" placeholder=\"RSSI Schwellwert\" style=\"width:100%;padding:10px;border:1px solid #ddd;border-radius:6px;margin-bottom:15px\"><div style=\"display:flex;gap:10px\"><button onclick=\"addKnownDevice(\\'' + address + '\\')\" style=\"flex:1;background:#007aff;color:white;border:none;padding:12px;border-radius:6px;cursor:pointer\">Hinzufügen</button><button onclick=\"closeDialog()\" style=\"flex:1;background:#8e8e93;color:white;border:none;padding:12px;border-radius:6px;cursor:pointer\">Abbrechen</button></div></div>';"
        "document.body.appendChild(dialog);"
        "document.getElementById('device-comment').focus();"
        "window.currentDialog = dialog;"
        "}"
        "function addKnownDevice(address) {"
        "const comment = document.getElementById('device-comment').value;"
        "const threshold = document.getElementById('device-threshold').value;"
        "if (!comment.trim()) {"
        "showNotification('Bitte Kommentar eingeben', true);"
        "return;"
        "}"
        "const params = new URLSearchParams({ address: address, known: true, comment: comment, rssiThreshold: threshold });"
        "fetch('/api/device/known?' + params, { method: 'POST' }).then(response => response.json()).then(data => {"
        "if (data.status === 'success') {"
        "showNotification(data.message);"
        "loadDevices();"
        "closeDialog();"
        "} else {"
        "showNotification(data.message, true);"
        "}"
        "}).catch(error => {"
        "showNotification('Fehler beim Hinzufügen', true);"
        "});"
        "}"
        "function closeDialog() {"
        "if (window.currentDialog) {"
        "document.body.removeChild(window.currentDialog);"
        "window.currentDialog = null;"
        "}"
        "}"
        "function refreshData() {"
        "loadDevices();"
        "showNotification('Daten aktualisiert');"
        "}"
        "function exportDevices() {"
        "window.location.href = '/api/export-devices-file';"
        "showNotification('Export gestartet...');"
        "}"
        "function importDevices(input) {"
        "const file = input.files[0];"
        "if (!file) return;"
        "const reader = new FileReader();"
        "reader.onload = function(e) {"
        "fetch('/api/import-devices-file', {"
        "method: 'POST',"
        "headers: { 'Content-Type': 'application/json' },"
        "body: e.target.result"
        "}).then(response => response.json()).then(data => {"
        "showNotification(data.message, data.status !== 'success');"
        "if (data.status === 'success') {"
        "loadDevices();"
        "}"
        "input.value = '';"
        "}).catch(error => {"
        "showNotification('Fehler beim Import', true);"
        "input.value = '';"
        "});"
        "};"
        "reader.readAsText(file);"
        "}"
        "function resetWiFi() {"
        "if (confirm('WiFi wirklich zurücksetzen? Das Gerät wird neu gestartet.')) {"
        "fetch('/api/wifi/reset', { method: 'POST' }).then(response => response.json()).then(data => {"
        "showNotification('WiFi wird zurückgesetzt...');"
        "}).catch(error => {"
        "showNotification('Reset wird durchgeführt...');"
        "});"
        "}"
        "}"
        "function resetBluetooth() {"
        "if (confirm('Bluetooth wirklich zurücksetzen? Alle temporären Scan-Daten gehen verloren.')) {"
        "fetch('/api/bluetooth/reset', { method: 'POST' }).then(response => response.json()).then(data => {"
        "showNotification(data.message, data.status !== 'success');"
        "if (data.status === 'success') {"
        "loadDevices();"
        "}"
        "}).catch(error => {"
        "showNotification('Bluetooth wird zurückgesetzt...', true);"
        "});"
        "}"
        "}"
        "function resetSystem() {"
        "if (confirm('System wirklich neu starten? Alle gescannten Geräte gehen verloren.')) {"
        "fetch('/api/system/reset', { method: 'POST' }).then(response => response.json()).then(data => {"
        "showNotification('System wird neu gestartet...');"
        "}).catch(error => {"
        "showNotification('System wird neu gestartet...');"
        "});"
        "}"
        "}"
        "document.getElementById('clear-log-btn').onclick = clearOutputLog;"
        "loadDevices();"
        "</script>"
        "</body>"
        "</html>"
    );
}