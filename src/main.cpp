#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "nvs_config.h"
#include "ntp.h"
#include "led.h"
#include "sensors.h"
#include "valves.h"
#include "wifi_manager.h"
#include "mqtt_client.h"

static NVSConfig     g_cfg;
static unsigned long g_last_publish[4]  = {0, 0, 0, 0};  // per-zone timers
static unsigned long g_last_tank        = 0;
static unsigned long g_last_heartbeat   = 0;
static unsigned long g_last_safety      = 0;
static bool          g_prev_valve[4]    = {false, false, false, false};

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("\n[NestGrow] Boot...");

    led_init();
    led_set_state(LedState::AP_MODE);

    // Init WiFi chip first — MAC address needed for device name generation
    WiFi.mode(WIFI_STA);

    sensors_init();
    valves_init();
    nvs_load(g_cfg);

    if (!g_cfg.configured) {
        Serial.println("[NestGrow] Primo avvio — captive portal attivo");
        wifi_start_ap(g_cfg);
        return;
    }

    led_set_state(LedState::CONNECTING);

    if (!wifi_connect_station(g_cfg)) {
        Serial.println("[NestGrow] WiFi fallito — avvio AP di emergenza");
        wifi_start_ap(g_cfg);
        return;
    }

    syncNTP();  // sincronizza orologio dopo connessione WiFi

    mqtt_init(g_cfg);
    led_set_state(LedState::CONNECTED);

    Serial.println("NestGrow v" FIRMWARE_VERSION " — Operativo");
    Serial.printf("Device : %s\n", g_cfg.device_name);
    Serial.printf("IP     : %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Broker : %s:%d\n", g_cfg.mqtt_host, g_cfg.mqtt_port);
}

void loop() {
    led_update();

    // ── AP / captive-portal mode ─────────────────────────────────────────────
    if (wifi_is_ap_mode()) {
        wifi_loop();
        return;
    }

    // ── WiFi watchdog ────────────────────────────────────────────────────────
    if (WiFi.status() != WL_CONNECTED) {
        led_set_state(LedState::CONNECTING);
        WiFi.reconnect();
        delay(500);
        return;
    }

    // ── MQTT loop + reconnect ─────────────────────────────────────────────────
    mqtt_loop();
    led_set_state(mqtt_is_connected() ? LedState::CONNECTED : LedState::ERROR);

    unsigned long now = millis();

    // ── Per-zone soil publish (ogni zona ha il suo timer) ─────────────────────
    for (int i = 0; i < 4; i++) {
        unsigned long interval = (unsigned long)mqtt_get_zone_interval(i + 1);
        if (now - g_last_publish[i] >= interval) {
            g_last_publish[i] = now;
            mqtt_publish_soil(i + 1, sensors_read_soil(i + 1));
        }
    }

    // ── Tank publish (timer fisso PUBLISH_INTERVAL_MS) ────────────────────────
    if (now - g_last_tank >= PUBLISH_INTERVAL_MS) {
        g_last_tank = now;
        mqtt_publish_tank(sensors_read_float());
    }

    // ── Heartbeat ogni 60 s ───────────────────────────────────────────────────
    if (now - g_last_heartbeat >= HEARTBEAT_INTERVAL_MS) {
        g_last_heartbeat = now;
        mqtt_publish_heartbeat();
    }

    // ── Valve safety check + state-change publish ogni 1 s ───────────────────
    if (now - g_last_safety >= 1000UL) {
        g_last_safety = now;

        float tank = sensors_read_float();
        valves_safety_check(tank);

        for (int i = 1; i <= 4; i++) {
            bool cur = valves_is_open(i);
            if (cur != g_prev_valve[i - 1]) {
                g_prev_valve[i - 1] = cur;
                int sec = cur ? valves_seconds_remaining(i) : 0;
                mqtt_publish_valve_state(i, cur, sec);
            }
        }
    }
}
