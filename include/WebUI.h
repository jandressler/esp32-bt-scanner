#pragma once
#include <Arduino.h>
#include "DeviceModeManager.h"

/**
 * WebUI - Modular Web Interface Generator
 * 
 * Strukturiert in klare Komponenten:
 * - Styles: CSS für alle UI-Elemente
 * - Structure: HTML-Grundstruktur
 * - Sections: Wiederverwendbare UI-Abschnitte
 * - Scripts: JavaScript-Funktionalität
 */
class WebUI {
public:
    // Main entry points
    static String generateMainHTML();
    static String generateModeSetupHTML(const String& beaconName, const BeaconConfig& beaconConfig);
    
private:
    // ========== CSS Styles ==========
    static String buildStyles();
    
    // ========== HTML Structure ==========
    static String buildHTMLHeader();
    static String buildHTMLFooter();
    
    // ========== Main UI Sections ==========
    static String buildPageHeader();
    static String buildStatusBar();
    static String buildControlsSection();
    static String buildDevicesSection();
    static String buildKnownDevicesSection();
    static String buildOutputLogSection();
    static String buildNotificationContainer();
    
    // ========== JavaScript ==========
    static String buildMainScript();
    
    // JavaScript - Core Functions
    static String buildCoreFunctions();
    static String buildDevicesFunctions();
    static String buildKnownDevicesFunctions();
    static String buildOutputLogFunctions();
    static String buildActionFunctions();
    static String buildInitFunction();
    
    // ========== Mode Setup UI (Beacon/WiFi/AP) ==========
    static String buildModeSetupStyles();
    static String buildModeSetupHeader();
    static String buildTabButtons();
    static String buildBeaconTab(const String& beaconName, const BeaconConfig& beaconConfig);
    static String buildWiFiTab();
    static String buildAPTab();
    static String buildModeSetupScript();
};
