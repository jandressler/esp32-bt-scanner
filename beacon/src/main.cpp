#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEAdvertising.h>
#include <esp_gap_ble_api.h>

#ifndef LED_BUILTIN_PIN
#define LED_BUILTIN_PIN 8
#endif

#ifndef LED_ACTIVE_LOW
#define LED_ACTIVE_LOW 1
#endif

#ifndef DEFAULT_ADV_INTERVAL_MS
#define DEFAULT_ADV_INTERVAL_MS 400
#endif

#ifndef DEFAULT_BASE_NAME
#define DEFAULT_BASE_NAME "BT-beacon"
#endif

static String makeBeaconNameFromBleAddr()
{
  BLEAddress bleAddr = BLEDevice::getAddress();
  const uint8_t* bda = (const uint8_t*)bleAddr.getNative();
  char suffix[5];
  snprintf(suffix, sizeof(suffix), "%02X%02X", bda[4], bda[5]);
  String name = String(DEFAULT_BASE_NAME) + String("_") + String(suffix);
#ifdef CUSTOM_BEACON_NAME
  name = String(CUSTOM_BEACON_NAME);
#endif
  return name;
}

static uint16_t toAdvUnits(uint32_t ms)
{
  // BLE adv interval units are 0.625ms; allowed 20ms..10240ms
  uint32_t minMs = 20;
  uint32_t maxMs = 10240;
  uint32_t clamped = (ms < minMs) ? minMs : (ms > maxMs ? maxMs : ms);
  // Round to nearest unit
  uint32_t units = (uint32_t)((clamped * 1000) / 625);
  if (units < 0x0020) units = 0x0020;     // 20ms
  if (units > 0x4000) units = 0x4000;     // 10.24s
  return (uint16_t)units;
}

static inline void ledOn()
{
  if (LED_ACTIVE_LOW)
    digitalWrite(LED_BUILTIN_PIN, LOW);
  else
    digitalWrite(LED_BUILTIN_PIN, HIGH);
}

static inline void ledOff()
{
  if (LED_ACTIVE_LOW)
    digitalWrite(LED_BUILTIN_PIN, HIGH);
  else
    digitalWrite(LED_BUILTIN_PIN, LOW);
}

static BLEAdvertising* advertising = nullptr;

void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN_PIN, OUTPUT);
  ledOff();

  // Lower CPU frequency to reduce heat without sleep
  setCpuFrequencyMhz(80);

  // Init BLE with base name; we'll set the final name after init using BLE address
  BLEDevice::init(DEFAULT_BASE_NAME);
  advertising = BLEDevice::getAdvertising();

  // Standard flags + complete local name
  BLEAdvertisementData advData;
  advData.setFlags(0x06); // LE General Discoverable Mode, BR/EDR Not Supported
  String finalName = makeBeaconNameFromBleAddr();
  advData.setName(finalName.c_str());
  // Also set GAP device name to match advertising name
  esp_ble_gap_set_device_name(finalName.c_str());

  // Provide scan response with the name as well for scanners relying on it
  BLEAdvertisementData scanResp;
  scanResp.setName(finalName.c_str());

  advertising->setAdvertisementData(advData);
  advertising->setScanResponseData(scanResp);

  // Set interval
  uint16_t intervalUnits = toAdvUnits(DEFAULT_ADV_INTERVAL_MS);
  advertising->setMinInterval(intervalUnits);
  advertising->setMaxInterval(intervalUnits);

  // We control advertising per cycle: start/stop around the blink
  advertising->stop();

  // Print BLE address-derived name for verification
  BLEAddress bleAddr = BLEDevice::getAddress();
  Serial.printf("Beacon name: %s\n", finalName.c_str());
  Serial.printf("BLE Addr: %s\n", bleAddr.toString().c_str());

  // Visible startup test: three quick blinks
  for (int i = 0; i < 3; ++i) {
    ledOn();
    delay(120);
    ledOff();
    delay(120);
  }
}

void loop()
{
  // Sleep first for the configured interval minus the 100ms LED pulse
  uint32_t sleepMs = (DEFAULT_ADV_INTERVAL_MS > 100) ? (DEFAULT_ADV_INTERVAL_MS - 100) : DEFAULT_ADV_INTERVAL_MS;
  // Light sleep for energy saving while keeping CPU at 80MHz during active time
  esp_sleep_enable_timer_wakeup((uint64_t)sleepMs * 1000ULL);
  esp_light_sleep_start();

  // Small stabilization delay after wakeup
  delay(5);

  // Start advertising right after wakeup
  advertising->start();
  // Allow a few milliseconds to queue the first advertising event
  delay(10);

  // Mark the advertising window with LED
  ledOn();
  delay(100);
  ledOff();

  // Stop advertising until next cycle
  advertising->stop();
}
