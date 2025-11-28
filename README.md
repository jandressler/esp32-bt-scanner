# ESP32-C3 Bluetooth Scanner & Home Automation Gateway

[![PlatformIO](https://img.shields.io/badge/PlatformIO-ready-orange)](https://platformio.org/)
[![ESP32-C3](https://img.shields.io/badge/ESP32-C3-blue)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](https://opensource.org/licenses/MIT)
[![RAM Usage](https://img.shields.io/badge/RAM-27.2%25-green)](docs/performance.md)
[![Flash Usage](https://img.shields.io/badge/Flash-47.5%25-yellow)](docs/performance.md)

Kompaktes ESP32-C3 System zum Scannen von Bluetooth-Geräten mit Web-Oberfläche und einfachen HTTP-Integrationen (z.B. Loxone). Speicher-statisch, ohne serielle Laufzeit-Logs.

## 🎯 Hauptfunktionen
### 🌐 Modernes Web-Interface
- **Responsive Design**: Mobile-First Ansatz, Touch-optimiert
- **Aktualisierung**: On-Demand (Button/Seitenreload), kein Auto-Polling
- **Moderne Oberfläche**: Schlanke UI mit klarer Typografie
- **Dialog-Overlays**: Geräteverwaltung mit RSSI-Threshold-Einstellung
- **Payload-Hex-Dump**: Vollständige BLE-Advertising-Daten für Entwickler

### 📊 Geräteverwaltung
- **Aktive Geräte**: Bis zu 32 gleichzeitig gescannte BLE-Geräte (LRU-Ersetzung)
- **Bekannte Geräte**: Bis zu 200 persistente Geräte mit Kommentaren (32 Zeichen)
- **RSSI-Schwellenwerte**: Individuell pro Gerät einstellbar (-60 bis -90 dBm)
- **Timeout**: 2 Minuten ohne Signal = automatisch inaktiv
### 💾 Backup & Restore
- **JSON-Export**: Download der bekannten Geräte
- **Import**: Browser-native File-API
- **Konfliktfrei**: Bestehende Geräte werden aktualisiert
- **Validierung**: Basis-JSON-Parsing ohne Schema-Validierung
- **Hardware WiFi-Reset**: Boot-Button (GPIO9) 3s drücken → WiFi-Credentials löschen




### Relais-Modul Kompatibilität
- **3.3V Logic-Level**: Direkt ESP32-C3 kompatibel
- **Optokoppler-Isolation**: Empfohlen für 24V-Industrie-Anwendungen
- **Max. Schaltleistung**: Abhängig vom verwendeten Relais-Modul
- **Anwendungen**: Türöffner, Garagentore, Beleuchtung, Schütze

## 🔧 Installation & Konfiguration

### Voraussetzungen
```bash
# PlatformIO Core Installation
pip install platformio

# Oder PlatformIO IDE für VS Code
# https://platformio.org/install/ide?install=vscode
```

### 1. WiFi-Setup via Captive Portal ⭐ **ZERO CONFIG**

**Komplett automatisches Setup - KEINE Code-Änderungen nötig!**

```mermaid
graph TD
    A[ESP32 hochladen] --> B[Access Point startet]
    B --> C[Smartphone zu ESP32-BT-Scanner verbinden]
    C --> D[Captive Portal öffnet automatisch]
    D --> E[WLAN auswählen + Passwort]
    E --> F[Save klicken]
    F --> G[ESP32 neustart + WLAN-Verbindung]
    G --> H[Web-Interface unter IP verfügbar]
```

**Schritt-für-Schritt:**
1. **Firmware flashen**: `platformio run --target upload`
2. **WLAN öffnen**: ESP32 startet als `ESP32-BT-Scanner` (OHNE Passwort)
3. **Verbindung**: Smartphone/Laptop zum Access Point verbinden
4. **Portal öffnet automatisch**: 
   - **Automatisch**: Bei den meisten Geräten öffnet sich das Setup-Portal automatisch
   - **Manuell**: Falls nicht, Browser öffnen und `http://192.168.4.1` eingeben
5. **Konfiguration**: WLAN auswählen, Passwort eingeben, "Save"
6. **Fertig**: ESP32 startet neu, Web-Interface unter neuer IP verfügbar

**💡 Tipp**: Wenn das Captive Portal nicht automatisch öffnet, einfach im Browser `192.168.4.1` aufrufen.

**WiFi-Reset**: Boot-Button (GPIO9) 3 Sekunden drücken → Neukonfiguration

### 2. Kompilierung & Upload

```bash
# Projekt klonen (falls noch nicht vorhanden)
git clone https://github.com/jandressler/esp32-bt-scanner.git
cd esp32-bt-scanner

# Dependencies automatisch installiert via platformio.ini
platformio run

# Upload auf ESP32-C3 (automatischer Port-Detection)
platformio run --target upload

# Serielle Konsole: Standard-Logs sind deaktiviert (Speicher-Optimierung)
platformio device monitor --baud 115200
```

### 3. Erste Einrichtung & Konfiguration

```mermaid
graph LR
    A[Web-Interface öffnen] --> B[BLE-Scan läuft automatisch]
    B --> C[Geräte als bekannt markieren]
    C --> D[RSSI-Schwellenwerte einstellen]
    D --> E[Export erstellen]
    E --> F[System läuft produktiv]
```

1. **Web-Interface**: `http://[ESP32_IP]` öffnen (solange kein WLAN verbunden ist erscheint automatisch die Setup-Seite, danach die Hauptseite)
2. **Geräte-Scan**: Läuft automatisch, BLE-Geräte erscheinen in Liste
3. **Bekannte Geräte**: Gewünschte Geräte als "bekannt" markieren
4. **RSSI-Tuning**: Schwellenwerte pro Gerät einstellen (-60 bis -90 dBm)
5. **Backup**: Export erstellen für Datensicherung
6. **Produktiv**: System läuft 24/7 mit automatischem Monitoring

## 🌐 Web-Interface Features

### Dashboard-Übersicht
```
┌─ Status-Bar ─────────────────────────────────────────┐
│ WiFi: ✅ Connected │ Geräte: 5/32 │ Output: 🟢 AN     │
│ Uptime: 2d 14h 32m │ Scan: ✅ Active │ RAM: 21.5%    │
└──────────────────────────────────────────────────────┘

┌─ Bekannte Geräte ──────────────────────────────────────┐
│ 📱 iPhone 15 Pro        │ 🟢 Anwesend │ -65 dBm      │
│ 🎧 AirPods Pro          │ 🔴 Abwesend │ ---          │
│ ⌚ Apple Watch Ultra     │ 🟡 Schwach  │ -89 dBm      │
└────────────────────────────────────────────────────────┘

┌─ Output-Log (letzte 30 Einträge) ──────────────────────┐
│ gerade eben │ iPhone 15 Pro │ 🟢 AN  │ Gerät erkannt │
│ vor 2m      │ AirPods Pro   │ 🔴 AUS │ Signal zu schwach
│ vor 5m      │ System        │ 🟢 AN  │ Test-Schaltung│
└────────────────────────────────────────────────────────┘
```

### Erweiterte Funktionen
- **Payload-Analyse**: Hex-Dump mit Hersteller-Dekodierung
- **Filter & Sortierung**: Nach Name, RSSI, Hersteller, Status
- **Export/Import**: JSON-basierte Backup-/Restore-Funktionen
- **System-Management**: WiFi/Bluetooth/System Reset-Buttons
- **Aktualisierung**: On-Demand via Button oder Seitenreload

## 🔗 REST-API Dokumentation

### 📱 Device Management API

```http
GET  /api/devices
Content-Type: application/json

Response:
{
  "status": "success",
  "devices": [
    {
      "address": "aa:bb:cc:dd:ee:ff",
      "name": "iPhone 15 Pro",
      "rssi": -65,
      "known": true,
      "active": true,
      "lastSeenRelative": "vor 5s",
      "manufacturer": "Apple Inc.",
      "payloadHex": "0201061AFF...",
      "comment": "Mein iPhone",
      "rssiThreshold": -70,
      "proximityStatus": "green"
    }
  ],
  "knownDevices": [
    {
      "address": "aa:bb:cc:dd:ee:ff",
      "comment": "Mein iPhone",
      "rssiThreshold": -70,
      "present": true,
      "proximityStatus": "green"
    }
  ]
}
```

```http
POST /api/device/known?address={MAC}&known={true|false}&comment={TEXT}&rssiThreshold={-60..-90}
Content-Type: application/json

Response:
{
  "status": "success",
  "message": "Gerät als bekannt markiert"
}
```

### 📊 System Status API

```http
GET /api/status
Response:
{
  "uptime": "2h 15m",
  "devices_ever": 42,
  "devices": 5,
  "known": 3,
  "present": 2,
  "wifi_connected": true,
  "wifi_ssid": "HomeNetwork",
  "wifi_rssi": -45,
  "wifi_ip": "192.168.1.100",
  "wifi_gateway": "192.168.1.1",
  "wifi_subnet": "255.255.255.0",
  "wifi_dns": "192.168.1.1",
  "wifi_mode": "Station",
  "heap_free": 228456,
  "scanning": true,
  "outputActive": true
}
```

### 📝 Output Log API

```http
GET  /api/output-log
Content-Type: application/json

Response:
{
  "outputLog": [
    {
      "timestamp": 1696518234,
      "deviceAddress": "aa:bb:cc:dd:ee:ff",
      "deviceName": "iPhone 15 Pro",
      "comment": "Mein iPhone",
      "outputState": true,
      "reason": "Gerät erkannt",
      "formattedTime": "gerade eben"
    }
  ]
}
```

```http
POST /api/output-log/clear    # Log löschen
POST /api/output-log/test     # Test-Eintrag erstellen
```

### 💾 Backup & Restore API

```http
GET  /api/export-devices-file
Content-Type: application/json
Content-Disposition: attachment; filename="bt_scanner_backup_2024-10-05-14-30-15.json"

Response:
{
  "devices": [
    {
      "address": "aa:bb:cc:dd:ee:ff",
      "name": "iPhone 15 Pro",
      "comment": "Mein iPhone",
      "rssiThreshold": -70,
      "isKnown": true
    }
  ],
  "backup_info": {
    "version": "1.0",
    "device_count": 1,
    "export_time": 1696518234,
    "description": "BT Scanner Backup - Bekannte Geräte"
  }
}
```

```http
POST /api/import-devices-file
Content-Type: application/json
Body: {JSON backup data}

Response:
{
  "status": "success",
  "message": "3 Geräte erfolgreich importiert"
}
```

### 🔧 System Management API

```http
POST /api/system/reset         # ESP32 Neustart
POST /api/wifi/reset          # WiFi-Credentials löschen
POST /api/bluetooth/reset     # Bluetooth-Stack neustart
```

## 🏠 Loxone Home Automation Integration

### Miniserver HTTP-Eingänge

```http
GET /loxone/presence
Content-Type: text/plain
Response: "present" | "absent"

GET /loxone/status  
Content-Type: text/plain
Response: "online" | "error"

GET /loxone/device?address={MAC}
Content-Type: text/plain
Response: "present" | "absent" | "unknown"
```

### Loxone Konfiguration

**1. Virtueller HTTP-Eingang erstellen:**
```
Name: BT_Scanner_Presence
URI: http://192.168.1.100/loxone/presence
Abfrageintervall: 10 Sekunden
Timeout: 5 Sekunden
Parser: Text-Parser
Erwartete Werte: "present", "absent"
```

**2. Funktionsblock-Programmierung:**
```
┌─ HTTP-Eingang ─┐    ┌─ Text-Vergleich ─┐    ┌─ Aktion ─┐
│ BT_Scanner     │───▶│ = "present"      │───▶│ Licht AN │
│ _Presence      │    │                  │    └──────────┘
└────────────────┘    └──────────────────┘
                              │
                              ▼
                      ┌─ Sonst-Aktion ─┐
                      │ Sicherheitsmodus│
                      └─────────────────┘
```

**3. Erweiterte Anwendungen:**
```bash
# System-Überwachung
curl http://192.168.1.100/loxone/status
# Bei "error" → Benachrichtigung senden

# Personenspezifische Automation
curl "http://192.168.1.100/loxone/device?address=aa:bb:cc:dd:ee:ff"
# Individuelle Beleuchtungs-/Heizungssteuerung

# Anwesenheitserkennung
curl http://192.168.1.100/loxone/presence
# Haupt-Anwesenheitsstatus für Gebäude-Automation
```

## ⚡ Hardware-Integration Details

### LED-Steuerung (GPIO 8)
```cpp
// ESP32-C3 onboard RGB LED - Invertierte Logik
#define LED_BUILTIN_PIN 8

// LED-Status-Mapping
digitalWrite(LED_BUILTIN_PIN, LOW);   // LED AN  (present = true)
digitalWrite(LED_BUILTIN_PIN, HIGH);  // LED AUS (present = false)

// Blink-Pattern für Feedback
void blinkLED(int times, int delayMs) {
    for(int i = 0; i < times; i++) {
        digitalWrite(LED_BUILTIN_PIN, LOW);   // AN
        delay(delayMs);
        digitalWrite(LED_BUILTIN_PIN, HIGH);  // AUS
        delay(delayMs);
    }
}
```

### Relais-Steuerung (GPIO 4)
```cpp
// Standard Logic-Level Relais
#define RELAY_OUTPUT_PIN 4

// Relais-Steuerung
digitalWrite(RELAY_OUTPUT_PIN, HIGH);  // Relais AN  (present = true)
digitalWrite(RELAY_OUTPUT_PIN, LOW);   // Relais AUS (present = false)

// Synchrone LED+Relais Steuerung
void setPresenceOutput(bool devicePresent) {
    digitalWrite(LED_BUILTIN_PIN, devicePresent ? LOW : HIGH);    // LED
    digitalWrite(RELAY_OUTPUT_PIN, devicePresent ? HIGH : LOW);   // Relais
    
    // Log-Eintrag bei Statuswechsel
    if (devicePresent != lastOutputState) {
        deviceManager.logOutputChange(
            triggerDevice, triggerName, triggerComment, 
            devicePresent, 
            devicePresent ? "Gerät erkannt" : "Kein bekanntes Gerät in Reichweite"
        );
        lastOutputState = devicePresent;
    }
}
```

### Anwesenheitserkennung-Logic
```cpp
// Proximity-Detection Algorithm
bool isDevicePresent = false;
for (int i = 0; i < deviceManager.getKnownCount(); i++) {
    char* deviceMAC = deviceManager.getKnownMACs()[i];
    int threshold = deviceManager.getKnownRSSIThresholds()[i];
    
    SafeDevice* device = findDevice(deviceMAC);
    if (device && device->isActive && device->rssi >= threshold) {
        isDevicePresent = true;
        setPresenceOutput(true, device->address, device->name, device->comment);
        break;
    }
}

if (!isDevicePresent) {
    setPresenceOutput(false, "", "", "");
}
```

## 🏭 24V Industrie-Integration (optional)

### Professionelle 24V-Setup-Architektur

```
┌─ 24V DC Versorgung ─┐     ┌─ ESP32-C3 System ─┐     ┌─ 24V Load ─┐
│ Netzteil/Hutschiene │────▶│ DC-DC Wandler      │     │ Türöffner  │
│ 24V/2A             │     │ 24V→3.3V/1A        │     │ Schütz     │
└────────────────────┘     │ Galv. getrennt     │     │ Ventil     │
                          └────────────────────┘     │ Motor      │
                                    │                └────────────┘
                          ┌─ GPIO 4 (3.3V) ─┐             ▲
                          │                 │             │
                          ▼                 ▼             │
                   ┌─ Optokoppler ─┐ ┌─ Relais-Board ─┐   │
                   │ PC817         │ │ 24V/10A        │───┘
                   │ 3.3V → 24V    │ │ Solid State    │
                   └───────────────┘ └────────────────┘
```

### Vorteile der 24V-Integration
- ✅ **Galvanische Trennung**: ESP32-C3 Schutz vor Industriespannungen
- ✅ **EMV-Konformität**: Optokoppler filtert elektromagnetische Störungen
- ✅ **Hohe Schaltlasten**: 24V Relais für Motoren, Schütze (bis 10A)
- ✅ **Fernverkabelung**: 24V über große Distanzen ohne Spannungsabfall
- ✅ **Industriestandard**: Kompatibel mit SPS, Schaltschränken, Hutschienen

### Komponenten-Empfehlungen
```yaml
DC-DC Wandler: TRACO TEN 3-2411WI (24V→3.3V, 1A, 1.5kV isolation)
Optokoppler: PC817 (CTR >50%, 35V, 50mA)
Relais-Modul: Crydom D24xx (24V Solid State, 3-32V control)
Verkabelung: 0.75mm² für Steuerleitungen, 1.5mm² für Lastkreise
Sicherung: Automaten-Sicherung B2A für 24V-Kreis
```

## 📊 Performance & Ressourcen-Monitoring

### Aktuelle System-Auslastung
```yaml
RAM-Nutzung:
  Gesamt: 320KB
  Verwendet: 89.236 bytes (27.2%)
  Verfügbar: 238.444 bytes (72.8%)
  Stack: ~8KB (reserviert)
  Heap: ~260KB (dynamisch)

Flash-Nutzung:
  Gesamt: 4MB
  Firmware: 1.495.176 bytes (47.5%)
  Verfügbar: 1.650.552 bytes (52.5%)
  OTA-Reserve: ~1.5MB (für Updates)
  Filesystem: ~500KB (für zukünftige Features)

CPU-Performance:
  Architektur: RISC-V 160MHz
  BLE-Scan: ~2% CPU (2s alle 10s)
  Web-Server: ~1% CPU (idle)
  JSON-Processing: ~5% CPU (bei API-Calls)
  Watchdog: <0.1% CPU (Hardware-Timer)
```

### Timing-Charakteristiken
```yaml
Scan-Zyklus: 10s (2s scan + 8s pause)
API-Response: <50ms (typ. 20-30ms)
Web-Page-Load: <200ms (kompressed HTML)
Relais-Switching: <1ms (GPIO direct)
System-Startup: <5s (WiFi + BLE init)
Recovery-Time: ~3s (nach Watchdog-Reset)
```

### Speicher-Management
```cpp
// Device Arrays (statisch alloziert)
SafeDevice devices[MAX_DEVICES];              // 32 * 280 bytes = 8.9KB
char knownMACs[MAX_KNOWN][18];               // 200 * 18 bytes = 3.6KB
char knownComments[MAX_KNOWN][MAX_COMMENT_LENGTH]; // 200 * 32 bytes = 6.4KB
int knownRSSIThresholds[MAX_KNOWN];          // 200 * 4 bytes = 0.8KB
OutputLogEntry outputLog[MAX_OUTPUT_LOG_ENTRIES]; // 30 * 120 bytes = 3.6KB

// Total Static Memory: ~23KB
// Dynamic Memory (JSON, Buffers): ~15KB
// Network Buffers: ~20KB
// Bluetooth Stack: ~25KB
```

## 🔍 Troubleshooting & Debug

### WiFi-Probleme
```bash
# Symptom: Captive Portal nicht erreichbar
1. ESP32 Reset-Button drücken
2. LED-Status prüfen: Dauerhaft AN = AP-Modus aktiv
3. WLAN-Liste aktualisieren: "ESP32-BT-Scanner" suchen
4. Näher zum ESP32 gehen (< 2m Entfernung)
5. 2.4GHz Band aktiviert? (nicht 5GHz)

# Symptom: WLAN-Verbindung instabil
1. Boot-Button (GPIO9) 3s drücken → WiFi-Reset
2. Captive Portal erneut durchlaufen
3. Router-Firmware aktualisieren
4. Kanal-Interferenzen prüfen (Kanal 1, 6, 11 bevorzugt)

# Debug via Serial Monitor
platformio device monitor --baud 115200
# Hinweis: Standard-Code erzeugt keine Serial-Ausgaben; Diagnose über /api/status und /health
```

### Bluetooth-Probleme
```bash
# Symptom: Keine BLE-Geräte gefunden
1. Web-Interface: "Bluetooth Reset" klicken
2. System-Reset über API: curl -X POST http://IP/api/system/reset
3. iOS: Bluetooth kurz aus/ein schalten
4. Android: Flugmodus aus/ein schalten

# Symptom: Bekannte Geräte nicht erkannt
1. RSSI-Schwellenwerte prüfen (-60 zu hoch → -80)
2. Gerät näher zum ESP32 bewegen
3. Output-Log prüfen auf Status-Wechsel
4. Device-Timeout: 2min ohne Signal = "weg"

# BLE-Debug via Serial Monitor
# Hinweis: BLE-Ereignisse erscheinen nicht seriell; Statuswechsel über Output-Log /api/output-log
```

### Web-Interface-Probleme
```bash
# Symptom: Seite lädt nicht
1. IP-Adresse korrekt? (im Captive Portal angezeigt)
2. Browser-Cache leeren: Ctrl+Shift+R (Chrome/Firefox)
3. Anderer Browser testen (Safari, Edge)
4. Firewall-Blockade prüfen (Port 80)

# Symptom: Daten nicht aktuell
1. "Aktualisieren" Button klicken
2. Seite neu laden (F5)
3. JavaScript-Console prüfen (F12)
4. API direkt testen: curl http://IP/api/devices

# Mobile-Optimierung
- Touch-Targets >44px
- Responsive Breakpoints: 320px, 768px, 1024px
- Offline-Funktionen für langsame Verbindungen
```

### Hardware-Diagnose
```bash
# LED-Status interpretieren
Dauerhaft AUS:   Kein bekanntes Gerät in Reichweite
Dauerhaft AN:    Mindestens ein bekanntes Gerät nah genug (Threshold erfüllt)
Blinkt schnell:  WiFi-Verbindungsaufbau
Blinkt 3x:       WiFi-Reset erkannt

# GPIO-Testing via API
curl -X POST http://IP/api/output-log/test
# Erwartung: LED blinkt 5x, Relais klickt

# Relais-Diagnose
1. Verkabelung prüfen: GPIO 4 → Relais IN
2. Spannung messen: 3.3V bei present=true
3. Relais-Spezifikation: 3.3V Trigger-Level
4. Last-Strom prüfen: <10mA für GPIO
```

### Performance-Monitoring
```bash
# RAM-Monitoring via API
curl http://IP/api/status | jq '.system.freeHeap'
# Normal: >200KB verfügbar

# Uptime-Tracking
curl http://IP/api/status | jq '.system.uptime'
# Target: >7 Tage ohne Reset

# Scan-Performance
# Normal: Device-Liste alle 10s aktualisiert
# Problem: Keine Updates >30s = BLE-Fehler
```

## 📄 Lizenz & Support

### MIT License
```
Copyright (c) 2024 Jan Dressler

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
```

### Support & Community
- 🐛 **Bug Reports**: [GitHub Issues](https://github.com/jandressler/esp32-bt-scanner/issues)
- 💡 **Feature Requests**: [GitHub Discussions](https://github.com/jandressler/esp32-bt-scanner/discussions)
- 📖 **Documentation**: [Wiki](https://github.com/jandressler/esp32-bt-scanner/wiki)
- 💬 **Community Support**: (Discord Link Placeholder)
- 🎯 **Professional Support**: [kontakt@jandressler.de](mailto:kontakt@jandressler.de)

### Contributing Guidelines
```markdown
1. Fork the repository
2. Create feature branch: git checkout -b feature/amazing-feature
3. Follow coding standards (clang-format provided)
4. Add tests for new functionality
5. Update documentation (README, Wiki, Code comments)
6. Submit Pull Request with detailed description
```
