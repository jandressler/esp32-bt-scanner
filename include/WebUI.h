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
    
    // Public build functions for chunked response
    static String buildStyles();
    static String buildHTMLHeader();
    static String buildHTMLFooter();
    static String buildPageHeader();
    static String buildControlsSection();
    static String buildDevicesSection();
    static String buildKnownDevicesSection();
    static String buildOutputLogSection();
    static String buildNotificationContainer();
    static String buildMainScript();
    
private:
    // ========== Main UI Sections ==========
    static String buildStatusBar();
    
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
