# ESP32-C3 BLE Beacon

A minimal BLE advertiser for ESP32-C3. Advertises a device name and blinks the LED on each cycle. No Wi‑Fi, no UI.

## Features
- BLE advertising with adjustable interval (default 500 ms)
- LED blink on each cycle (GPIO 8 by default)
- Device name defaults to `BT-beacon_<XXXX>` with MAC suffix; override possible
- Light sleep between cycles for lower power

## Configure
- Change defaults via `platformio.ini` build flags:
  - `-DLED_BUILTIN_PIN=8`
  - `-DDEFAULT_ADV_INTERVAL_MS=500`
  - `-DDEFAULT_BASE_NAME=\"BT-beacon\"`
- Optional full name override: add `-DCUSTOM_BEACON_NAME=\"YourName\"`

## Build & Upload
```
platformio run -d beacon
platformio run -d beacon --target upload
```

## Notes
- Advertising interval uses BLE units (0.625 ms) and clamps to valid ranges.
- Light sleep is used between cycles; BLE advertising remains active.
