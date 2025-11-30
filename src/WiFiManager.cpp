/**
 * @file WiFiManager.cpp
 * @brief Implementation der WiFi-Verwaltung
 */

#include "WiFiManager.h"

WiFiManager::WiFiManager() 
    : isInSecureMode(false), buttonPressStart(0), buttonPressed(false), useStaticIP(false), dnsServer(nullptr) {
}

WiFiManager::~WiFiManager() {
    if (dnsServer) {
        dnsServer->stop();
        delete dnsServer;
        dnsServer = nullptr;
    }
}

bool WiFiManager::begin() {
    // Configure reset button
    pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
    
    loadSettings();
    updateSecurityStatus();
    
    // Try to connect to saved WiFi
    if (savedSSID.length() > 0) {
        if (connectToWiFi()) {
            isInSecureMode = true;
            return true;
        }
    }
    
    // Start Access Point for setup
    return startAccessPoint(savedAPPassword);
}

bool WiFiManager::connectToWiFi() {
    if (savedSSID.length() == 0) return false;
    
    WiFi.mode(WIFI_STA);
    
    // Konfiguriere statische IP falls aktiviert
    if (useStaticIP) {
        if (!WiFi.config(staticIP, gateway, subnet, dns)) {
            
            useStaticIP = false;
        }
    }
    
    WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
    
    // Wait up to 20 seconds for connection
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
        delay(500);
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        isInSecureMode = true;
        
        // Stop DNS Server when connected to WiFi
        if (dnsServer) {
            dnsServer->stop();
            delete dnsServer;
            dnsServer = nullptr;
            
        }
        
        return true;
    }
    
    return false;
}

bool WiFiManager::startAccessPoint(const String& apPassword) {
    WiFi.mode(WIFI_AP);
    
    bool success;
    if (apPassword.length() >= 8) {
        // Secure AP with password
        success = WiFi.softAP(WIFI_MANAGER_AP_NAME, apPassword.c_str());
        isInSecureMode = true;
    } else {
        // Open AP for initial setup
        success = WiFi.softAP(WIFI_MANAGER_AP_NAME);
        isInSecureMode = false;
    }
    
    if (success) {
        // Start DNS Server for Captive Portal
        if (!dnsServer) {
            dnsServer = new DNSServer();
        }
        dnsServer->start(53, "*", WiFi.softAPIP());
        
    }
    
    return success;
}

void WiFiManager::disconnect() {
    WiFi.disconnect();
    WiFi.softAPdisconnect();
}

bool WiFiManager::saveWiFiCredentials(const String& ssid, const String& password) {
    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.end();
    
    savedSSID = ssid;
    savedPassword = password;
    
    return true;
}

bool WiFiManager::saveAPPassword(const String& password) {
    if (password.length() < 8) return false;
    
    preferences.begin("wifi", false);
    preferences.putString("ap_password", password);
    preferences.end();
    
    savedAPPassword = password;
    updateSecurityStatus();
    
    return true;
}

void WiFiManager::clearAllSettings() {
    preferences.begin("wifi", false);
    preferences.clear();
    preferences.end();
    
    savedSSID = "";
    savedPassword = "";
    savedAPPassword = "";
    isInSecureMode = false;
}

bool WiFiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

bool WiFiManager::isInAPMode() const {
    return WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA;
}

String WiFiManager::getLocalIP() const {
    if (isConnected()) {
        return WiFi.localIP().toString();
    }
    return "";
}

String WiFiManager::getAPIP() const {
    if (isInAPMode()) {
        return WiFi.softAPIP().toString();
    }
    return "";
}

void WiFiManager::checkResetButton() {
    bool currentButtonState = !digitalRead(MODE_BUTTON_PIN);  // Pull-up inverted
    
    if (currentButtonState && !buttonPressed) {
        buttonPressed = true;
        buttonPressStart = millis();
    } else if (!currentButtonState && buttonPressed) {
        buttonPressed = false;
        unsigned long pressDuration = millis() - buttonPressStart;
        
        if (pressDuration >= MODE_BUTTON_DURATION_MS) {
            performReset();
        }
    }
}

void WiFiManager::performReset() {
    clearAllSettings();
    
    // Visual feedback - blink 3 times
    for (int i = 0; i < 6; i++) {
        digitalWrite(LED_BUILTIN_PIN, i % 2);
        delay(200);
    }
    
    ESP.restart();
}

void WiFiManager::loadSettings() {
    preferences.begin("wifi", true);  // read-only
    
    savedSSID = preferences.getString("ssid", "");
    savedPassword = preferences.getString("password", "");
    savedAPPassword = preferences.getString("ap_password", "");
    
    // Statische IP laden
    useStaticIP = preferences.getBool("use_static_ip", false);
    if (useStaticIP) {
        String ipStr = preferences.getString("static_ip", "");
        String gwStr = preferences.getString("gateway", "");
        String snStr = preferences.getString("subnet", "255.255.255.0");
        String dnsStr = preferences.getString("dns", "8.8.8.8");
        
        if (ipStr.length() > 0) {
            staticIP.fromString(ipStr);
            gateway.fromString(gwStr);
            subnet.fromString(snStr);
            dns.fromString(dnsStr);
        } else {
            useStaticIP = false;
        }
    }
    
    preferences.end();
}

bool WiFiManager::connectToWiFi(const String& ssid, const String& password) {
    if (ssid.length() == 0) {
        
        return false;
    }
    
    // Speichere Credentials
    if (!saveWiFiCredentials(ssid, password)) {
        
        return false;
    }
    
    // Stop DNS Server before switching to STA mode
    if (dnsServer) {
        dnsServer->stop();
        delete dnsServer;
        dnsServer = nullptr;
    }
    
    // Versuche Verbindung
    WiFi.mode(WIFI_STA);
    
    // Konfiguriere statische IP falls aktiviert
    if (useStaticIP) {
        if (!WiFi.config(staticIP, gateway, subnet, dns)) {
            
        }
    }
    
    WiFi.begin(ssid.c_str(), password.c_str());
    
    
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_TIMEOUT_MS) {
        delay(100);
        
    }
    
    
    if (WiFi.status() == WL_CONNECTED) {
        
        isInSecureMode = true;
        return true;
    } else {
        
        // Fallback zum AP-Modus
        startAccessPoint("");
        return false;
    }
}

bool WiFiManager::connectToWiFi(const String& ssid, const String& password, const String& ip, const String& gateway, const String& subnet, const String& dns) {
    if (ssid.length() == 0) {
        
        return false;
    }
    
    // Speichere Credentials
    if (!saveWiFiCredentials(ssid, password)) {
        
        return false;
    }
    
    // Speichere statische IP Config
    if (ip.length() > 0 && gateway.length() > 0) {
        if (!saveStaticIPConfig(ip, gateway, subnet, dns)) {
            
        }
    }
    
    // Stop DNS Server before switching to STA mode
    if (dnsServer) {
        dnsServer->stop();
        delete dnsServer;
        dnsServer = nullptr;
    }
    
    // Versuche Verbindung
    WiFi.mode(WIFI_STA);
    
    // Konfiguriere statische IP
    if (useStaticIP) {
        if (!WiFi.config(staticIP, this->gateway, this->subnet, this->dns)) {
            
        } else {
            
        }
    }
    
    WiFi.begin(ssid.c_str(), password.c_str());
    
    
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_TIMEOUT_MS) {
        delay(100);
        
    }
    
    
    if (WiFi.status() == WL_CONNECTED) {
        
        isInSecureMode = true;
        return true;
    } else {
        
        // Fallback zum AP-Modus
        startAccessPoint("");
        return false;
    }
}

bool WiFiManager::setAPPassword(const String& password) {
    if (password.length() < 8) {
        
        return false;
    }
    
    if (!saveAPPassword(password)) {
        
        return false;
    }
    
    savedAPPassword = password;
    isInSecureMode = true;
    
    
    return true;
}

void WiFiManager::resetWiFiSettings() {
    
    
    preferences.begin("wifi", false);
    preferences.clear();
    preferences.end();
    
    savedSSID = "";
    savedPassword = "";
    savedAPPassword = "";
    isInSecureMode = false;
    
    WiFi.disconnect(true);
    delay(100);
}

void WiFiManager::updateSecurityStatus() {
    // Gerät ist sicher wenn:
    // 1. WiFi Client-Verbindung aktiv ist, oder
    // 2. AP-Modus mit Passwort läuft
    isInSecureMode = isConnected() || (!savedAPPassword.isEmpty());
    
    
}

bool WiFiManager::saveStaticIPConfig(const String& ip, const String& gateway, const String& subnet, const String& dns) {
    preferences.begin("wifi", false);
    
    preferences.putBool("use_static_ip", true);
    preferences.putString("static_ip", ip);
    preferences.putString("gateway", gateway);
    preferences.putString("subnet", subnet.length() > 0 ? subnet : "255.255.255.0");
    preferences.putString("dns", dns.length() > 0 ? dns : "8.8.8.8");
    
    preferences.end();
    
    useStaticIP = true;
    staticIP.fromString(ip);
    this->gateway.fromString(gateway);
    this->subnet.fromString(subnet.length() > 0 ? subnet : "255.255.255.0");
    this->dns.fromString(dns.length() > 0 ? dns : "8.8.8.8");
    
    
    return true;
}

void WiFiManager::clearStaticIPConfig() {
    preferences.begin("wifi", false);
    
    preferences.putBool("use_static_ip", false);
    preferences.remove("static_ip");
    preferences.remove("gateway");
    preferences.remove("subnet");
    preferences.remove("dns");
    
    preferences.end();
    
    useStaticIP = false;
    
    
}

void WiFiManager::loop() {
    // Process DNS requests when in AP mode
    if (dnsServer && isInAPMode()) {
        dnsServer->processNextRequest();
    }
}