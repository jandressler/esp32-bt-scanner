/**
 * @file WiFiManager.h
 * @brief WiFi-Verbindungsmanagement
 * 
 * Diese Klasse verwaltet WiFi-Verbindungen, Access Point Setup
 * und WiFi-Konfiguration mit Sicherheitsfeatures.
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include "Config.h"

/**
 * @brief WiFi-Manager Klasse
 */
class WiFiManager {
private:
    Preferences preferences;
    String savedSSID;
    String savedPassword;
    String savedAPPassword;
    bool isInSecureMode;
    unsigned long buttonPressStart;
    bool buttonPressed;
    
    // Statische IP Konfiguration
    bool useStaticIP;
    IPAddress staticIP;
    IPAddress gateway;
    IPAddress subnet;
    IPAddress dns;
    
public:
    WiFiManager();
    ~WiFiManager();
    
    // Initialization
    bool begin();
    
    // WiFi Connection Management
    bool connectToWiFi();
    bool startAccessPoint(const String& apPassword = "");
    void disconnect();
    
    // Configuration Management
    bool saveWiFiCredentials(const String& ssid, const String& password);
    bool saveAPPassword(const String& password);
    void clearAllSettings();
    
    // Setup Methods
    bool connectToWiFi(const String& ssid, const String& password);
    bool connectToWiFi(const String& ssid, const String& password, const String& ip, const String& gateway, const String& subnet, const String& dns);
    bool setAPPassword(const String& password);
    void resetWiFiSettings();
    
    // Static IP Management
    bool saveStaticIPConfig(const String& ip, const String& gateway, const String& subnet, const String& dns);
    void clearStaticIPConfig();
    bool isUsingStaticIP() const { return useStaticIP; }
    
    // Status
    bool isConnected() const;
    bool isInAPMode() const;
    bool isSecure() const { return isInSecureMode; }
    String getLocalIP() const;
    String getAPIP() const;
    
    // Reset functionality
    void checkResetButton();
    void performReset();
    
    // Getters
    String getSSID() const { return savedSSID; }
    String getAPPassword() const { return savedAPPassword; }
    
private:
    void loadSettings();
    void updateSecurityStatus();
};

#endif // WIFI_MANAGER_H