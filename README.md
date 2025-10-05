# ESP32-C3 Bluetooth Scanner & Home Automation Gateway# ESP32-C3 Bluetooth Scanner & Home Automation Gateway



Ein umfassendes IoT-System für den ESP32-C3, das Bluetooth-Geräte scannt, eine Web-Oberfläche bereitstellt und als Gateway für Home Automation Systeme wie Loxone fungiert.Ein umfassendes IoT-System für den ESP32-C3, das Bluetooth-Geräte scannt, eine Web-Oberfläche bereitstellt und als Gateway für Home Automation Systeme wie Loxone fungiert.



## 🎯 Hauptfunktionen## 🎯 Hauptfunktionen



### 🔍 **Bluetooth Low Energy (BLE) Scanner**### 🔍 **Bluetooth Low Energy (BLE) Scanner**

- Kontinuierliches Scannen nach BLE-Geräten (2s scan, 8s pause)- Kontinuierliches Scannen nach BLE-Geräten (2s scan, 8s pause)

- Erweiterte Payload-Analyse mit Herstellererkennung- Erweiterte Payload-Analyse mit Herstellererkennung

- Automatische Geräteerkennung (Apple, Samsung, Google, Xiaomi, etc.)- Automatische Geräteerkennung (Apple, Samsung, Google, Xiaomi, etc.)

- RSSI-basierte Anwesenheitserkennung mit konfigurierbaren Schwellenwerten- RSSI-basierte Anwesenheitserkennung mit konfigurierbaren Schwellenwerten

- Intelligente Gerätefilterung und -verwaltung (2min Timeout)- Intelligente Gerätefilterung und -verwaltung (2min Timeout)



### 📶 **WiFiManager Captive Portal** ⭐### 📶 **WiFiManager Captive Portal** ⭐

- **Plug & Play Setup**: Keine Konfiguration in Code nötig- **Plug & Play Setup**: Keine Konfiguration in Code nötig

- **Offenes WLAN**: "ESP32-BT-Scanner" ohne Passwort für einfaches Setup- **Offenes WLAN**: "ESP32-BT-Scanner" ohne Passwort für einfaches Setup

- **Captive Portal**: Automatisches Setup über Smartphone/Laptop- **Captive Portal**: Automatisches Setup über Smartphone/Laptop

- **WiFi-Reset**: Boot-Button 3 Sekunden drücken für Neukonfiguration- **WiFi-Reset**: Boot-Button 3 Sekunden drücken für Neukonfiguration

- **Intelligenter Modus**: Bluetooth-Scanning nur wenn WiFi verbunden- **Intelligenter Modus**: Bluetooth-Scanning nur wenn WiFi verbunden

- **2.4GHz Optimiert**: Bluetooth AUS während WiFi-Setup für Stabilität- **2.4GHz Optimiert**: Bluetooth AUS während WiFi-Setup für Stabilität



### 🐕 **Hardware Watchdog** ⭐### 🐕 **Hardware Watchdog** ⭐

- **Automatischer Neustart** bei System-Hängern (30s Timeout)- **Automatischer Neustart** bei System-Hängern (30s Timeout)

- **Robuste Überwachung**: Verhindert permanente Ausfälle- **Robuste Überwachung**: Verhindert permanente Ausfälle

- **API-Reset**: Fernwartung über `/api/system/reset`- **API-Reset**: Fernwartung über `/api/system/reset`

- **WiFi-Reset**: Gespeicherte Credentials löschen über `/api/wifi/reset`- **WiFi-Reset**: Gespeicherte Credentials löschen über `/api/wifi/reset`



### 🌐 **Modernes Web-Interface**### 🌐 **Modernes Web-Interface**

- Responsive Design für Desktop und Mobile- Responsive Design für Desktop und Mobile

- Echtzeit-Geräteliste mit detaillierten Informationen- Echtzeit-Geräteliste mit detaillierten Informationen

- **Output-Log**: 30 Einträge mit Schaltungsverlauf und Zeitstempel- **Output-Log**: 30 Einträge mit Schaltungsverlauf und Zeitstempel

- Modal-Dialoge für Payload-Analyse- Modal-Dialoge für Payload-Analyse

- Intuitive Geräteverwaltung mit Kommentaren und RSSI-Schwellenwerten- Intuitive Geräteverwaltung mit Kommentaren und RSSI-Schwellenwerten



### 🏠 **Home Automation Integration**### 🏠 **Home Automation Integration**

- **GPIO 4 Relais-Ausgang** für Hardware-Steuerung (Türöffner, Garagentore)- **GPIO 4 Relais-Ausgang** für Hardware-Steuerung (Türöffner, Garagentore)

- **LED-Statusanzeige** (GPIO 8) mit invertierter Logik- **LED-Statusanzeige** (GPIO 8) mit invertierter Logik

- **Synchrone LED+Relais-Steuerung** bei Geräte-Anwesenheitserkennung- **Synchrone LED+Relais-Steuerung** bei Geräte-Anwesenheitserkennung

- **Loxone-kompatible HTTP-API** für nahtlose Integration- **Loxone-kompatible HTTP-API** für nahtlose Integration

- **Output-Log-System**: Vollständige Nachverfolgung aller Schaltungen- **Output-Log-System**: Vollständige Nachverfolgung aller Schaltungen



### 💾 **Backup & Restore System**### 💾 **Backup & Restore System**

- **Echter Datei-Export**: JSON-Download mit Zeitstempel- **Echter Datei-Export**: JSON-Download mit Zeitstempel

- **Datei-Upload-Import**: Drag & Drop JSON-Wiederherstellung- **Datei-Upload-Import**: Drag & Drop JSON-Wiederherstellung

- Strukturierte Backup-Metadaten mit Versionierung- Strukturierte Backup-Metadaten mit Versionierung

- Merge-Funktion für konfliktfreie Imports- Merge-Funktion für konfliktfreie Imports



## 📋 Hardware-Anforderungen- **WiFi-Reset**: Gespeicherte Credentials löschen über `/api/reset-wifi`



### **ESP32-C3 DevKitM-1**### 🏠 **Known Devices Management**

- **CPU**: RISC-V 160MHz

- **RAM**: 320KB (ca. 21% genutzt)- Persistent gespeicherte Geräteliste### 🌐 **Modernes Web-Interface**

- **Flash**: 4MB (ca. 47.7% genutzt)

- **GPIO 8**: Eingebaute LED (invertierte Logik)- RSSI-Schwellenwerte pro Gerät konfigurierbar- Responsive Design für Desktop und Mobile

- **GPIO 4**: Relais-Ausgang (3.3V kompatibel)

- **GPIO 0**: Boot-Button (WiFi-Reset Funktion)- Benutzer-Kommentare für bessere Organisation- Echtzeit-Geräteliste mit detaillierten Informationen



### **Relais-Modul** (optional)- Import/Export-Funktionalität für Backups- Export/Import-Funktionalität für Gerätekonfigurationen

- 3.3V kompatibles Relais-Modul

- Anschluss an GPIO 4- Modal-Dialoge für Payload-Analyse

- Für Türöffner, Garagentore, Beleuchtung, etc.

## 📋 Hardware- Intuitive Geräteverwaltung mit Kommentaren

## 🔧 Installation & Konfiguration



### 1. **WiFi-Setup (Captive Portal)** ⭐ **EINFACH**

### **ESP32-C3 DevKitM-1**### 🏠 **Home Automation Integration**

**Automatisches Setup - KEINE Konfiguration in Code nötig!**

- **CPU**: RISC-V 160MHz- **Loxone-kompatible HTTP-API** für nahtlose Integration

```

1. ESP32-C3 mit USB verbinden und Firmware hochladen- **RAM**: 320KB SRAM - **GPIO 4 Relais-Ausgang** für Hardware-Steuerung (Türöffner, Garagentore)

2. ESP32 startet automatisch als Access Point "ESP32-BT-Scanner" (OHNE Passwort)

3. Mit Smartphone/Laptop zu "ESP32-BT-Scanner" verbinden  - **Flash**: 4MB- **LED-Statusanzeige** (GPIO 8) mit invertierter Logik

4. Captive Portal öffnet sich automatisch (oder http://192.168.4.1)

5. Gewünschtes WLAN auswählen und Passwort eingeben- **Bluetooth**: BLE 5.0- Synchrone LED+Relais-Steuerung bei Geräteerkennung

6. "Save" klicken - ESP32 startet neu und verbindet sich

7. Fertig! Web-Interface unter der angezeigten IP verfügbar- **WiFi**: 2.4GHz 802.11 b/g/n

```

### 💾 **Backup & Restore System**

**WiFi-Reset:** Boot-Button 3 Sekunden drücken → Neukonfiguration

### **GPIO-Belegung**- **Echter Datei-Export**: JSON-Download mit Zeitstempel

### 2. **Kompilieren & Hochladen**

- **GPIO 0**: Boot-Button (WiFi-Reset)- **Datei-Upload-Import**: Drag & Drop JSON-Wiederherstellung

```bash

# Projekt kompilieren- **GPIO 8**: Status-LED (invertiert)- Strukturierte Backup-Metadaten mit Versionierung

platformio run

- **GPIO 4**: Relais-Ausgang- Merge-Funktion für konfliktfreie Imports

# Auf ESP32-C3 hochladen

platformio run --target upload



# Seriellen Monitor starten (optional)## 🔧 Installation## 📋 Hardware-Anforderungen

platformio device monitor

```



### 3. **Erste Einrichtung**### **PlatformIO**### **ESP32-C3 DevKitM-1**



1. **Captive Portal**: WiFi-Setup erfolgt automatisch beim ersten Start```bash- **CPU**: RISC-V 160MHz

2. **Web-Interface öffnen**: `http://[ESP32_IP]` (IP wird im Captive Portal angezeigt)

3. **Geräte scannen** und als "bekannt" markieren# Repository klonen- **RAM**: 320KB (ca. 19% genutzt)

4. **RSSI-Schwellenwerte** pro Gerät einstellen (-60 bis -90 dBm)

5. **Export erstellen** für Backupgit clone <repository-url>- **Flash**: 4MB (ca. 46% genutzt)



## 🌐 Web-Interface Featurescd bt_scanner- **GPIO 8**: Eingebaute LED (invertierte Logik)



### **Hauptseite** (`/`)- **GPIO 4**: Relais-Ausgang (3.3V kompatibel)

- **Status-Bar**: WiFi, Geräteanzahl, Scan-Status, Output-Status, Uptime

- **Geräteliste**: Alle gefundenen BLE-Geräte mit Details# Kompilieren und hochladen

- **Payload-Analyse**: Detaillierte Herstellerinformationen 

- **Geräteverwaltung**: Kommentare und RSSI-Schwellenwertepio run -t upload### **Relais-Modul** (optional)

- **Output-Log**: Verlauf aller Ausgangsschaltungen (30 Einträge)

```- 3.3V kompatibles Relais-Modul

### **Geräte-Details**

- **MAC-Adresse**: Eindeutige Bluetooth-Identifikation- Anschluss an GPIO 4

- **Gerätename**: Automatisch erkannt oder manuell

- **Hersteller**: Automatische Erkennung via Payload-Analyse### **Erstes Setup**- Für Türöffner, Garagentore, etc.

- **RSSI-Wert**: Aktuelle Signalstärke mit Proximity-Status

- **Letzter Kontakt**: Zeitstempel der letzten Erkennung1. ESP32 mit USB verbinden und Code hochladen

- **Payload-Hex**: Rohe BLE-Advertising-Daten

2. WiFi-Netzwerk "ESP32-BT-Scanner" suchen (offen)## 🔧 Installation & Konfiguration

### **Output-Log**

- **30 Einträge**: Vollständiger Schaltungsverlauf3. Captive Portal öffnet automatisch

- **Zeitstempel**: "vor 5s", "vor 2m", "gerade eben"  

- **Geräte-Info**: Name und MAC-Adresse des auslösenden Geräts4. WLAN-Credentials eingeben### 1. **WiFi-Setup (Captive Portal)** ⭐ **EINFACH**

- **Status**: 🟢 AN / 🔴 AUS mit Grund der Schaltung

- **Automatische Bereinigung**: Alte Einträge werden überschrieben5. Gerät startet automatisch und beginnt Bluetooth-Scan



## 🔗 REST-API Endpunkte6. Web-Interface über IP-Adresse aufrufen**Automatisches Setup - KEINE Konfiguration in Code nötig!**



### **Standard Web-API**



```http## 🌐 Web-Interface```

GET  /                           # Haupt-Web-Interface

GET  /api/devices               # Alle BLE-Geräte (JSON)1. ESP32-C3 mit USB verbinden und Firmware hochladen

GET  /api/status                # System-Status (WiFi, Uptime, etc.)

POST /api/scan                  # Manueller BLE-Scan### **Zugriff**2. ESP32 startet automatisch als Access Point "ESP32-BT-Scanner" (OHNE Passwort)

POST /api/device/known          # Gerät als bekannt markieren

     ?address=aa:bb:cc:dd:ee:ff- Browser öffnen: `http://<ESP32-IP>`3. Mit Smartphone/Laptop zu "ESP32-BT-Scanner" verbinden  

     &known=true/false

     &comment=Kommentar- Standard-Port: 804. Captive Portal öffnet sich automatisch (oder http://192.168.4.1)

     &rssiThreshold=-70

```5. Gewünschtes WLAN auswählen und Passwort eingeben



### **System-Management APIs**### **Hauptfunktionen**6. "Save" klicken - ESP32 startet neu und verbindet sich



```http- **Status-Bar**: Scan-Status, Geräteanzahl, Uptime, RAM7. Fertig! Web-Interface unter der angezeigten IP verfügbar

POST /api/wifi/reset            # WiFi-Credentials löschen

POST /api/system/reset          # System-Neustart- **Device-Liste**: Alle erkannten Geräte mit Proximity-Status```

POST /api/bluetooth/reset       # Bluetooth-Neustart

```- **Known Devices**: Persistent gespeicherte Geräte verwalten



### **Output-Log APIs**- **Reset-Buttons**: Getrennte Buttons für WiFi/BT/System**WiFi-Reset:** Boot-Button 5 Sekunden drücken → Neukonfiguration



```http

GET  /api/output-log            # Schaltungs-Verlauf (JSON)

POST /api/output-log/clear      # Log löschen### **API-Endpoints**### **📱 Troubleshooting WiFiManager**

POST /api/output-log/test       # Test-Eintrag erstellen

``````



### **Export/Import APIs**GET  /api/devices     # Alle Geräte**Problem: Access Point instabil / Handy fliegt raus:**



```httpGET  /api/status      # System-Status- **Näher zum ESP32**: AP-Reichweite ist begrenzt (~2-3 Meter)

GET  /api/export-devices-file   # Datei-Download (JSON)

POST /api/import-devices-file   # Datei-Upload (Multipart)POST /api/known       # Gerät hinzufügen/entfernen- **2.4GHz prüfen**: Handy muss 2.4GHz unterstützen

```

POST /api/reset-wifi  # WiFi-Reset- **5 Minuten warten**: Längeres Timeout für Stabilität

### **🏠 Loxone Home Automation API**

POST /api/reset-bt    # Bluetooth-Reset  - **Desktop-Browser**: Falls Handy-Browser Probleme macht

```http

GET /loxone/presence            # Anwesenheitsstatus (text/plain)POST /api/reset-sys   # System-Reset

GET /loxone/status              # System-Status (text/plain)  

GET /loxone/device?address=MAC  # Spezifisches Gerät (text/plain)```**Problem: "Configure WiFi" Button funktioniert nicht:**

```

- **Desktop-Modus aktivieren** im Handy-Browser

#### **Loxone API Details**

## 🔧 Konfiguration- **Direct Link**: http://192.168.4.1 direkt aufrufen

**Anwesenheit abfragen:**

```bash- **Cache leeren**: Browser-Cache löschen

curl http://192.168.1.100/loxone/presence

# Antwort: "present" oder "absent"### **Config.h Einstellungen**- **Alternative**: Laptop/PC für Setup verwenden

```

```cpp

**System-Status:**

```bash// Scan-Parameter**Problem: Captive Portal öffnet nicht automatisch:**

curl http://192.168.1.100/loxone/status

# Antwort: "online" oder "error"#define BT_SCAN_DURATION_MS 2000      // 2s scannen- **Manuell öffnen**: http://192.168.4.1 im Browser

```

#define BT_PAUSE_DURATION_MS 8000     // 8s pause- **WLAN-Einstellungen**: "Anmelden" Button in WLAN-Settings

**Spezifisches Gerät:**

```bash#define DEVICE_TIMEOUT_MS 120000      // 2min timeout- **Mobile Daten aus**: Handy soll nur ESP32-WLAN nutzen

curl http://192.168.1.100/loxone/device?address=aa:bb:cc:dd:ee:ff

# Antwort: "present" oder "absent" oder "unknown"

```

// WiFi-Einstellungen  ### 2. **Manuelle WiFi-Konfiguration** (optional)

## ⚡ Hardware-Integration

#define WIFI_TIMEOUT_MS 10000         // 10s Verbindungsversuch

### **LED-Steuerung (GPIO 8)**

- **Invertierte Logik**: LOW = AN, HIGH = AUS#define WIFI_RESET_BUTTON_DURATION_MS 3000  // 3s Button-PressFalls gewünscht, können Sie `include/Config.h` trotzdem bearbeiten:

- **Statusanzeigen**:

  - **AUS**: Kein bekanntes Gerät in der Nähe

  - **AN**: Bekannte Geräte anwesend (über RSSI-Schwellenwert)

  - **Blinken**: WLAN-Verbindung wird hergestellt// System```cpp



### **Relais-Steuerung (GPIO 4)**#define WATCHDOG_TIMEOUT_MS 30000     // 30s Watchdog// WiFiManager wird für Setup verwendet - diese sind nur Fallback

- **Normale Logik**: HIGH = AN, LOW = AUS

- **Synchron mit LED**: Parallel geschaltet bei Anwesenheitserkennung#define MAX_DEVICES 8                 // Max Device-Liste#define WIFI_SSID "IHR_WLAN_NAME"

- **Anwendungen**: Türöffner, Garagentore, Beleuchtung, Schütze

- **3.3V Pegel**: Kompatibel mit Standard-Relais-Modulen#define MAX_KNOWN 3                   // Max Known Devices#define WIFI_PASSWORD "IHR_WLAN_PASSWORT"



### **Anwesenheitserkennung-Logik**```

```cpp

// Ein bekanntes Gerät löst Schaltung aus wenn:// GPIO-Pins

// 1. Gerät ist als "bekannt" markiert

// 2. Gerät ist aktiv (letzter Scan < 2min)  ### **Debug-Modi** (für Entwicklung)#define LED_BUILTIN_PIN 8      // Eingebaute LED

// 3. RSSI >= eingestellter Schwellenwert (z.B. -70 dBm)

```cpp#define RELAY_OUTPUT_PIN 4     // Relais-Ausgang

void setPresenceOutput(bool devicePresent, const char* triggerDevice, 

                      const char* triggerName, const char* triggerComment) {#define DEBUG_ENABLED false          // Production: deaktiviert

    // LED: ESP32-C3 invertierte Logik

    digitalWrite(LED_BUILTIN_PIN, devicePresent ? LOW : HIGH);#define DEBUG_BLUETOOTH false        // BT-Debug// Watchdog-Einstellungen

    

    // Relais: Normale Logik  #define DEBUG_WIFI false            // WiFi-Debug#define WATCHDOG_TIMEOUT_SEC 30   // 30 Sekunden Timeout

    digitalWrite(RELAY_OUTPUT_PIN, devicePresent ? HIGH : LOW);

    ```#define WATCHDOG_ENABLED true     // Watchdog aktivieren

    // Log-Eintrag erstellen (nur bei Statuswechsel)

    deviceManager.logOutputChange(triggerDevice, triggerName, 

                                 triggerComment, devicePresent, 

                                 devicePresent ? "Gerät erkannt" : "Kein bekanntes Gerät in Reichweite");## 📊 System-Status// Bluetooth-Einstellungen  

}

```#define BLE_SCAN_TIME 10       // Scan-Dauer in Sekunden



## 🏠 Loxone-Integration### **Memory Usage**#define BLE_SCAN_INTERVAL 30   // Intervall zwischen Scans



### **Miniserver-Konfiguration**- **RAM**: ~19.4% (63KB von 320KB)#define MAX_DEVICES 50         // Maximale Anzahl Geräte



**Virtueller HTTP-Eingang erstellen:**- **Flash**: ~47.6% (1.5MB von 3.1MB)#define MAX_KNOWN_DEVICES 20   // Maximale bekannte Geräte

```

Name: BT_Scanner_Presence```

URI: http://192.168.1.100/loxone/presence

Abfrageintervall: 10 Sekunden### **Performance**

Timeout: 5 Sekunden

```- **Scan-Zyklus**: 10 Sekunden total### 3. **Kompilieren & Hochladen**



**Programm-Baustein:**- **Webserver**: Async, nicht blockierend

```

WENN BT_Scanner_Presence = "present"- **JSON-API**: ArduinoJson v7.4.2```bash

DANN Aktion_bei_Anwesenheit (Licht an, Heizung hoch, etc.)

SONST Aktion_bei_Abwesenheit (Sicherheitsmodus, Energiesparen)- **Stabilität**: Hardware Watchdog (30s)# Projekt kompilieren

```

pio run

### **Erweiterte Loxone-Nutzung**

## 🐛 Troubleshooting

**System-Überwachung:**

```# Auf ESP32-C3 hochladen

URI: http://192.168.1.100/loxone/status

Erwartung: "online"### **WiFi-Probleme**pio run --target upload

Bei Fehler: Benachrichtigung senden

```- Boot-Button 3s drücken für WiFi-Reset



**Individuelle Personen-Erkennung:**- Captive Portal neu durchlaufen# Seriellen Monitor starten

```

URI: http://192.168.1.100/loxone/device?address=aa:bb:cc:dd:ee:ff- 2.4GHz WLAN verwenden (nicht 5GHz)pio device monitor

Für spezifische Personen-basierte Automatisierung

``````



## 💾 Backup-System### **Bluetooth-Probleme**  



### **Export-Format (JSON)**- Reset-Bluetooth Button im Web-Interface### 4. **Erste Einrichtung**



```json- System-Reset über Web-Interface

{

  "devices": [- Hardware-Reset über Reset-Button1. **Captive Portal**: WiFi-Setup erfolgt automatisch beim ersten Start

    {

      "address": "aa:bb:cc:dd:ee:ff",2. **Web-Interface öffnen**: `http://[ESP32_IP]` (IP wird im Captive Portal angezeigt)

      "name": "iPhone 15 Pro",

      "comment": "Mein Handy",### **Web-Interface**3. **Geräte scannen** und als "bekannt" markieren

      "rssiThreshold": -60,

      "isKnown": true- Cache leeren (Ctrl+F5)4. **Export erstellen** für Backup

    }

  ],- Korekte IP-Adresse überprüfen

  "backup_info": {

    "version": "1.0",- JSON-Parsing-Fehler in Browser-Console prüfen## 🌐 Web-Interface Features

    "device_count": 1,

    "export_time": 12345678,

    "description": "BT Scanner Backup - Bekannte Geräte"

  }## 📝 Entwicklung### **Hauptseite** (`/`)

}

```- **Geräteliste**: Alle gefundenen BLE-Geräte mit Details



### **Export-Prozess**### **Architektur**- **Payload-Analyse**: Detaillierte Herstellerinformationen 

1. **Klick auf "Export"** → Automatischer JSON-Download

2. **Dateiname**: `bt_scanner_backup_YYYY-MM-DD-HH-mm-ss.json`- **Modular**: BluetoothScanner, DeviceManager, WiFiManager, WebServerManager- **Geräteverwaltung**: Kommentare und RSSI-Schwellwerte

3. **Inhalt**: Alle bekannten Geräte mit Metadaten

- **Non-blocking**: Async Web-Server- **Export/Import**: Vollständige Backup-Funktionalität

### **Import-Prozess**

1. **Klick auf "Import"** → Datei-Auswahl-Dialog- **Memory-safe**: Strukturierte Daten, Buffer-Overflow-Schutz

2. **JSON-Datei auswählen** → Automatischer Upload

3. **Validierung**: Format-Prüfung vor Import- **Robust**: Watchdog, Auto-Reset, Error-Handling### **Geräte-Details**

4. **Merge**: Neue Geräte werden hinzugefügt (keine Duplikate)

- **MAC-Adresse**: Eindeutige Bluetooth-Identifikation

## 🔍 BLE Payload-Analyse

### **Code-Struktur**- **Gerätename**: Automatisch erkannt oder manuell

### **Unterstützte Hersteller**

- **Apple**: iPhone, iPad, Apple Watch, AirPods```- **Hersteller**: Automatische Erkennung via Payload-Analyse

- **Samsung**: Galaxy-Serie, Wearables

- **Google**: Pixel, Nest-Gerätesrc/- **RSSI-Wert**: Aktuelle Signalstärke

- **Microsoft**: Surface, Xbox-Controller

- **Xiaomi**: Mi-Serie, Redmi├── main_modular.cpp        # Hauptprogramm- **Letzter Kontakt**: Zeitstempel der letzten Erkennung

- **Und viele weitere...**

├── BluetoothScanner.cpp    # BLE-Scan-Logic- **Payload-Hex**: Rohe BLE-Advertising-Daten

### **Analyse-Features**

- **Automatische Herstellererkennung** via Company ID├── DeviceManager.cpp       # Geräteverwaltung

- **Gerätetyp-Identifikation** (iPhone, Galaxy, etc.)

- **Service-UUID-Dekodierung**├── WiFiManager.cpp         # WiFi-Verbindung## � REST-API Endpunkte

- **Payload-Hex-Dump** für manuelle Analyse

└── WebServerManager.cpp    # Web-Interface

## 🛠️ Troubleshooting

### **Standard Web-API**

### **WiFi-Probleme**

- **Boot-Button 3s drücken** für WiFi-Resetinclude/

- **Captive Portal neu durchlaufen**

- **2.4GHz WLAN verwenden** (nicht 5GHz)├── Config.h                # Zentrale Konfiguration```http

- **Näher zum ESP32** bei instabilem Access Point

├── BluetoothScanner.h      # BLE-Scanner HeaderGET  /                           # Haupt-Web-Interface

### **Bluetooth-Probleme**

- **Reset-Bluetooth Button** im Web-Interface├── DeviceManager.h         # Device-ManagementGET  /api/devices               # Alle BLE-Geräte (JSON)

- **System-Reset** über `/api/system/reset`

- **Hardware-Reset** über Reset-Button am ESP32├── WiFiManager.h           # WiFi-ManagerGET  /api/status                # System-Status



### **Web-Interface-Probleme**└── WebServerManager.h      # WebServer HeaderPOST /api/scan                  # Manueller BLE-Scan

- **Browser-Cache leeren** (Ctrl+F5)

- **Korrekte IP-Adresse** überprüfen```POST /api/clear                 # Geräteliste löschen

- **Port 80 Firewall-Freigabe** prüfen

POST /api/device/known          # Gerät als bekannt markieren

### **Relais schaltet nicht**

- **GPIO 4 Verkabelung** kontrollieren## 📅 Version & Updates     ?address=aa:bb:cc:dd:ee:ff

- **3.3V Relais-Kompatibilität** sicherstellen

- **Ausreichende Stromversorgung** gewährleisten     &known=true/false

- **Output-Log prüfen** ob Schaltbefehle ankommen

**Aktuelle Version**: v2.0.0       &comment=Kommentar

## 📊 System-Ressourcen

**Letzte Änderung**: 5. Oktober 2025     &rssiThreshold=-70

**Aktuelle Nutzung:**

- **RAM**: 68.812 bytes (21.0% von 320KB) ```

- **Flash**: 1.499.494 bytes (47.7% von 4MB)

- **Output-Log**: 30 Einträge (Ringpuffer)### **Recent Changes (v2.0)**

- **Known Devices**: Max. 3 Geräte

- **Device List**: Max. 8 aktive Geräte- ✅ Umfassende Payload-Datensammlung### **Export/Import API**

- **Watchdog**: 30 Sekunden Hardware-Überwachung

- ✅ Konsolidierte Status-Bar

**Performance:**

- **Scan-Zyklus**: 10 Sekunden (2s scan + 8s pause)- ✅ Manuelle Refresh-Buttons```http

- **Web-Response**: <100ms

- **API-Calls**: <50ms  - ✅ Getrennte Reset-FunktionenGET  /api/export-devices-file   # Datei-Download (JSON)

- **Relais-Switching**: <1ms

- **System-Startup**: <5 Sekunden- ✅ 2-Minuten Device-TimeoutPOST /api/import-devices-file   # Datei-Upload (Multipart)

- **WiFi-Setup**: ~30 Sekunden (einmalig)

- ✅ Proximity-Status (grün/gelb/rot)```

## 🔮 Zukünftige Erweiterungen

- ✅ Production-optimiert (Debug aus)

### **24V Industrie-Setup** ⚡

### **🏠 Loxone Home Automation API**

**Für professionelle 24V-Umgebungen:**

## 📄 Lizenz

```

24V DC Versorgung ──┐```http

                   │

    ┌──────────────▼──────────────┐MIT License - Siehe LICENSE-Datei für DetailsGET /loxone/presence            # Anwesenheitsstatus (text/plain)

    │  24V → 3.3V DC-DC Wandler   │  (z.B. LM2596, >85% Efficiency)GET /loxone/status              # System-Status (text/plain)  

    │  (min. 1A, 1kV isolation)   │GET /loxone/device?address=MAC  # Spezifisches Gerät (text/plain)

    └──────────────┬──────────────┘```

                   │ 3.3V

    ┌──────────────▼──────────────┐#### **Loxone API Details**

    │     ESP32-C3 DevKitM-1      │

    │  GPIO 4 ──┐                 │**Anwesenheit abfragen:**

    └───────────┼─────────────────┘```bash

                │ 3.3V Signalcurl http://192.168.1.100/loxone/presence

    ┌───────────▼─────────────────┐# Antwort: "present" oder "absent"

    │   Optokoppler-Relais        │  (PC817 + Relais-Board)```

    │   3.3V → 24V galv. getrennt │

    └───────────┬─────────────────┘**System-Status:**

                │ 24V Schaltausgang```bash

    ┌───────────▼─────────────────┐curl http://192.168.1.100/loxone/status

    │   24V Last (Türöffner,      │# Antwort: "online" oder "error"

    │   Schütz, Ventil, etc.)     │```

    └─────────────────────────────┘

```**Spezifisches Gerät:**

```bash

**Vorteile:**curl http://192.168.1.100/loxone/device?address=aa:bb:cc:dd:ee:ff

- ✅ **Galvanische Trennung**: Schutz des ESP32-C3# Antwort: "present" oder "absent" oder "unknown"

- ✅ **Industriestandard**: 24V-kompatible Ausgänge  ```

- ✅ **Hohe Lasten**: Schaltung von Motoren, Schützen

- ✅ **EMV-Schutz**: Optokoppler filtert Störungen## � Backup-System

- ✅ **Fernverkabelung**: 24V über große Distanzen

### **Export-Format (JSON)**

### **Geplante Features**

1. **MQTT-Integration**: Home Assistant Kompatibilität```json

2. **Mobile App**: Native iOS/Android App{

3. **Erweiterte Filter**: Zeit- und ortsbasierte Regeln  "devices": [

4. **Multi-Zone-Support**: Mehrere Bereiche überwachen    {

5. **Machine Learning**: Intelligente Anwesenheitsmuster      "address": "aa:bb:cc:dd:ee:ff",

      "name": "iPhone 15 Pro",

## 🚀 Produktive Bereitstellung      "comment": "Mein Handy",

      "rssiThreshold": -60,

### **Deployment-Checkliste**      "isKnown": true

- ✅ WiFi über Captive Portal konfiguriert    }

- ✅ Bekannte Geräte markiert und RSSI-Schwellenwerte gesetzt  ],

- ✅ Relais-Hardware getestet und Output-Log geprüft  "backup_info": {

- ✅ Loxone-API-Endpunkte validiert    "version": "1.0",

- ✅ Web-Interface Zugriff bestätigt    "device_count": 1,

- ✅ System-Monitoring eingerichtet    "export_time": 12345678,

    "description": "BT Scanner Backup - Bekannte Geräte"

### **Wartung & Updates**  }

```bash}

# System-Status prüfen```

curl http://[ESP32_IP]/loxone/status

### **Export-Prozess**

# Output-Log prüfen1. **Klick auf "Export"** → Automatischer JSON-Download

curl http://[ESP32_IP]/api/output-log2. **Dateiname**: `bt_scanner_backup_YYYY-MM-DD-HH-mm-ss.json`

3. **Inhalt**: Alle bekannten Geräte mit Metadaten

# Backup erstellen  

curl -O http://[ESP32_IP]/api/export-devices-file### **Import-Prozess**

1. **Klick auf "Import"** → Datei-Auswahl-Dialog

# Neue Firmware uploaden2. **JSON-Datei auswählen** → Automatischer Upload

platformio run --target upload3. **Validierung**: Format-Prüfung vor Import

4. **Merge**: Neue Geräte werden hinzugefügt (keine Duplikate)

# System-Reset nach Update

curl -X POST http://[ESP32_IP]/api/system/reset## � BLE Payload-Analyse

```

### **Unterstützte Hersteller**

## 📊 Projekt-Status- **Apple**: iPhone, iPad, Apple Watch, AirPods

- **Samsung**: Galaxy-Serie, Wearables

**🎉 PROJEKT ABGESCHLOSSEN - PRODUKTIONSREIF**- **Google**: Pixel, Nest-Geräte

- **Microsoft**: Surface, Xbox-Controller

✅ **Vollständige Bluetooth-Geräte-Erkennung**  - **Xiaomi**: Mi-Serie, Redmi

✅ **WiFiManager Captive Portal Setup**  - **Und viele weitere...**

✅ **Hardware Watchdog (30s Timeout)**  

✅ **GPIO 4 Relais-Ausgang mit LED-Synchronisation**  ### **Analyse-Features**

✅ **Output-Log-System (30 Einträge)**  - **Automatische Herstellererkennung** via Company ID

✅ **Loxone-kompatible HTTP-API**  - **Gerätetyp-Identifikation** (iPhone, Galaxy, etc.)

✅ **Export/Import Backup-System**  - **Service-UUID-Dekodierung**

✅ **Speicher-optimiert (21% RAM, 47.7% Flash)**  - **Payload-Hex-Dump** für manuelle Analyse

✅ **Web-Interface mit Echtzeit-Status**  

✅ **Automatische Uploads ohne Boot+Reset**  ## ⚡ Hardware-Integration



## 📄 Lizenz & Support### **LED-Steuerung (GPIO 8)**

- **Invertierte Logik**: LOW = AN, HIGH = AUS

**MIT-Lizenz** - Freie Nutzung und Modifikation- **Statusanzeigen**:

  - **Aus**: System aus oder Fehler

**Support:**  - **AN**: Bekannte Geräte anwesend

- GitHub Issues für Bugs  - **Blinken**: WLAN-Verbindung wird hergestellt

- PlatformIO Serial Monitor für Debug

- Web-Interface Output-Log für Troubleshooting### **Relais-Steuerung (GPIO 4)**

- **Normale Logik**: HIGH = AN, LOW = AUS

---- **Synchron mit LED**: Parallel geschaltet

- **Anwendungen**: Türöffner, Garagentore, Beleuchtung

**Entwickelt für professionelle Home Automation mit ESP32-C3** 🏠⚡🔧- **3.3V Pegel**: Kompatibel mit Standard-Relais-Modulen

### **Anwesenheitserkennung**
```cpp
void setPresenceOutput(bool devicePresent) {
    // LED: ESP32-C3 invertierte Logik
    digitalWrite(LED_BUILTIN_PIN, devicePresent ? LOW : HIGH);
    
    // Relais: Normale Logik
    digitalWrite(RELAY_OUTPUT_PIN, devicePresent ? HIGH : LOW);
}
```

## 🏠 Loxone-Integration

### **Miniserver-Konfiguration**

**Virtueller HTTP-Eingang erstellen:**
```
Name: BT_Scanner_Presence
URI: http://192.168.1.100/loxone/presence
Abfrageintervall: 10 Sekunden
```

**Programm-Baustein:**
```
WENN BT_Scanner_Presence = "present"
DANN Aktion_bei_Anwesenheit
SONST Aktion_bei_Abwesenheit
```

### **Erweiterte Loxone-Nutzung**

**System-Überwachung:**
```
URI: http://192.168.1.100/loxone/status
Erwartung: "online"
```

**Spezifische Geräte:**
```
URI: http://192.168.1.100/loxone/device?address=aa:bb:cc:dd:ee:ff
Für individuelle Personen-Erkennung
```

## 🛠️ Troubleshooting

### **Häufige Probleme**

**WLAN-Verbindung fehlgeschlagen:**
- Config.h: SSID/Passwort prüfen
- Router: 2.4GHz aktiviert?
- Serieller Monitor: Debug-Ausgaben

**Bluetooth funktioniert nicht:**
- ESP32-C3: BLE-Support aktiviert
- Geräte: Bluetooth eingeschaltet
- Scan-Intervall: Ausreichend lang

**Web-Interface nicht erreichbar:**
- IP-Adresse: Serieller Monitor
- Firewall: Port 80 geöffnet
- Browser: Cache leeren

**Relais schaltet nicht:**
- GPIO 4: Korrekt verdrahtet
- Relais-Modul: 3.3V kompatibel
- Power: Ausreichende Stromversorgung

### **Debug-Kommandos (Seriell)**

```
help      - Alle verfügbaren Kommandos
status    - System-Status anzeigen
devices   - BLE-Geräte auflisten
scan      - Manueller BLE-Scan
clear     - Geräteliste löschen
wifi      - WLAN-Informationen
restart   - System neu starten
```

## 📊 System-Ressourcen

**Aktuelle Nutzung (mit WiFiManager + Watchdog):**
- **RAM**: 64.276 bytes (19.6% von 320KB) 
- **Flash**: 1.589.512 bytes (50.5% von 4MB)
- **Heap**: Dynamisch überwacht
- **Stack**: Ausreichend reserviert
- **Upload**: Automatisch ohne Boot+Reset Sequenz
- **Watchdog**: 30 Sekunden Hardware-Überwachung

**Performance:**
- **Scan-Zyklus**: 30 Sekunden
- **Web-Response**: <100ms
- **API-Calls**: <50ms  
- **Relais-Switching**: <1ms
- **System-Startup**: <3 Sekunden
- **WiFi-Setup**: ~30 Sekunden (einmalig)

## 🔮 Zukünftige Erweiterungen

### **24V Industrie-Setup** ⚡🏭

**Für professionelle 24V-Umgebungen:**

```
24V DC Versorgung ──┐
                   │
    ┌──────────────▼──────────────┐
    │  24V → 3.3V DC-DC Wandler   │  (z.B. LM2596, Efficiency >85%)
    │  (min. 1A, isolation 1kV)   │
    └──────────────┬──────────────┘
                   │ 3.3V
    ┌──────────────▼──────────────┐
    │     ESP32-C3 DevKitM-1      │
    │  GPIO 4 ──┐                 │
    └───────────┼─────────────────┘
                │ 3.3V Signal
    ┌───────────▼─────────────────┐
    │   Optokoppler-Relais        │  (PC817 + Relais-Board)
    │   3.3V → 24V galv. getrennt │
    └───────────┬─────────────────┘
                │ 24V Schaltausgang
    ┌───────────▼─────────────────┐
    │   24V Last (Türöffner,      │
    │   Schütz, Ventil, etc.)     │
    └─────────────────────────────┘
```

**Vorteile:**
- ✅ **Galvanische Trennung**: Schutz des ESP32-C3
- ✅ **Industriestandard**: 24V-kompatible Ausgänge  
- ✅ **Hohe Lasten**: Schaltung von Motoren, Schützen
- ✅ **EMV-Schutz**: Optokoppler filtert Störungen
- ✅ **Fernverkabelung**: 24V über große Distanzen

**Empfohlene Komponenten:**
- **DC-DC Wandler**: LM2596-3.3V (1A, isoliert)
- **Optokoppler**: PC817 oder 4N35
- **Relais-Board**: 24V Spule, NO/NC Kontakte
- **Gehäuse**: DIN-Rail montierbar, IP65

### **Geplante Features**
1. **MQTT-Integration**: Home Assistant Kompatibilität
2. **Mobile App**: Native iOS/Android App
3. **Erweiterte Filter**: Zeit- und ortsbasierte Regeln
4. **Multi-Zone-Support**: Mehrere Bereiche überwachen
5. **Machine Learning**: Intelligente Anwesenheitsmuster

### **Hardware-Erweiterungen**
1. **Externe Antenne**: Verbesserte Reichweite
2. **Sensor-Integration**: Temperatur, Bewegung
3. **Display-Support**: OLED/E-Paper Status
4. **Battery-Backup**: USV-Funktionalität
5. **DIN-Rail Gehäuse**: Schaltschrank-Integration

## 🚀 Produktive Bereitstellung

### **Deployment-Checkliste**
- ✅ WiFi-Credentials in `Config.h` angepasst
- ✅ Bekannte Geräte konfiguriert und exportiert  
- ✅ Relais-Hardware getestet
- ✅ Loxone-API-Endpunkte validiert
- ✅ Web-Interface Zugriff bestätigt
- ✅ System-Monitoring eingerichtet

### **Wartung & Updates**
```bash
# System-Status prüfen
curl http://[ESP32_IP]/loxone/status

# Backup erstellen  
curl -O http://[ESP32_IP]/export

# Neue Firmware uploaden
pio run --target upload

# Reset nach Update
curl -X POST http://[ESP32_IP]/api/reset
```

### **Troubleshooting**
1. **Web-Interface nicht erreichbar**: WiFi-Verbindung prüfen
2. **Keine BLE-Geräte**: Bluetooth-Hardware-Reset via API
3. **Relais schaltet nicht**: GPIO 4 Verkabelung kontrollieren  
4. **Upload-Probleme**: Automatischer Upload ohne Boot+Reset

## 📊 Projekt-Status

**🎉 PROJEKT ABGESCHLOSSEN - PRODUKTIONSREIF**

✅ **Alle Hauptfunktionen implementiert**
✅ **Speicher-optimiert (12KB Flash + 136 Bytes RAM gespart)**
✅ **Serial Interface entfernt für Stabilität**
✅ **Automatische Uploads ohne manuelle Reset-Sequenz**
✅ **Vollständige Loxone-Integration**
✅ **Real-File Export/Import System**
✅ **Umfassende Payload-Analyse**
✅ **Hardware GPIO-Steuerung funktional**

## 📄 Lizenz & Support

**MIT-Lizenz** - Freie Nutzung und Modifikation

**Support-Kanäle:**
- GitHub Issues für Bugs
- Serieller Monitor für Debug
- Community-Forum für Fragen

---

**Entwickelt für professionelle Home Automation mit ESP32-C3** �⚡🔧