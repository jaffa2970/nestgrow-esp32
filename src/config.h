#pragma once

// Relay pins (inverted logic: LOW=ON, HIGH=OFF)
#define RELAY_1  25
#define RELAY_2  26
#define RELAY_3  27
#define RELAY_4  14

// Soil moisture ADC pins (ADC1 only — safe with WiFi)
#define SOIL_1   32
#define SOIL_2   33
#define SOIL_3   34  // input-only
#define SOIL_4   35  // input-only

// Float switch (external pullup)
#define FLOAT_SW 36  // input-only

// Built-in LED
#define LED_PIN  2

// Capacitive sensor V2.0 calibration (12-bit ADC)
#define SOIL_DRY  2800
#define SOIL_WET  1200

// MQTT
#define MQTT_PORT 1883

// Timing
#define PUBLISH_INTERVAL_MS   30000UL
#define HEARTBEAT_INTERVAL_MS 60000UL
#define VALVE_TIMEOUT_MS      300000UL
#define DEFAULT_INTERVAL_MS   30000UL  // default per-zone publish interval

// AP captive portal
#define AP_SSID_PREFIX "NestGrow-Setup-"
#define AP_IP_STR      "192.168.4.1"

#define FIRMWARE_VERSION "1.0.0"

// Uncomment for simulated sensor values
// #define DEBUG_SENSORS
