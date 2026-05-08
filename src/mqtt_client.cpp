#include "mqtt_client.h"
#include "config.h"
#include "nvs_config.h"
#include "valves.h"
#include "ntp.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

static WiFiClient    wc;
static PubSubClient  mc(wc);

static char s_device[32];
static char s_host[64];
static int  s_port;
static int  s_zone_interval[4];  // per-zone publish interval in ms

static unsigned long       s_last_reconnect = 0;
static const unsigned long RECONNECT_MS     = 5000UL;

// ── helpers ───────────────────────────────────────────────────────────────────

static void topic(char *buf, size_t len, const char *suffix) {
    snprintf(buf, len, "nestgrow/%s/%s", s_device, suffix);
}

// ── cmd/config handler ────────────────────────────────────────────────────────

static void handle_config_cmd(const char *buf) {
    JsonDocument doc;
    if (deserializeJson(doc, buf)) return;

    int  zona        = doc["zona"]         | -1;
    int  interval_ms = doc["intervallo_ms"] | 0;
    bool save_nvs    = doc["salva_nvs"]    | false;

    if (interval_ms <= 0) return;

    if (zona == 0) {
        for (int i = 0; i < 4; i++) s_zone_interval[i] = interval_ms;
    } else if (zona >= 1 && zona <= 4) {
        s_zone_interval[zona - 1] = interval_ms;
    } else {
        return;
    }

    if (save_nvs) {
        NVSConfig nc;
        nvs_load(nc);
        if (zona == 0) {
            for (int i = 0; i < 4; i++) nc.zona_interval[i] = interval_ms;
        } else {
            nc.zona_interval[zona - 1] = interval_ms;
        }
        nvs_save(nc);
    }

    // ACK
    char t[80], payload[96];
    topic(t, sizeof(t), "cmd/config/ack");
    snprintf(payload, sizeof(payload),
        "{\"zona\":%d,\"intervallo_ms\":%d,\"salvato\":%s}",
        zona, interval_ms, save_nvs ? "true" : "false");
    mc.publish(t, payload);

    Serial.printf("[MQTT] Config zona %d → %d ms (nvs=%s)\n",
        zona, interval_ms, save_nvs ? "si" : "no");
}

// ── incoming message callback ─────────────────────────────────────────────────

static void on_message(char *raw_topic, byte *payload, unsigned int length) {
    char buf[512];
    size_t n = length < sizeof(buf) - 1 ? length : sizeof(buf) - 1;
    memcpy(buf, payload, n);
    buf[n] = '\0';

    // cmd/reconfig
    char t_reconfig[80];
    topic(t_reconfig, sizeof(t_reconfig), "cmd/reconfig");
    if (strcmp(raw_topic, t_reconfig) == 0) {
        JsonDocument doc;
        if (deserializeJson(doc, buf)) return;
        NVSConfig nc;
        nvs_load(nc);
        const char *v;
        v = doc["ssid"]      | (const char*)nullptr; if (v) strlcpy(nc.ssid,      v, sizeof(nc.ssid));
        v = doc["password"]  | (const char*)nullptr; if (v) strlcpy(nc.password,  v, sizeof(nc.password));
        v = doc["mqtt_host"] | (const char*)nullptr; if (v) strlcpy(nc.mqtt_host, v, sizeof(nc.mqtt_host));
        nc.configured = true;
        nvs_save(nc);
        Serial.println("[MQTT] Reconfig — restarting");
        delay(500);
        ESP.restart();
        return;
    }

    // cmd/config
    char t_config[80];
    topic(t_config, sizeof(t_config), "cmd/config");
    if (strcmp(raw_topic, t_config) == 0) {
        handle_config_cmd(buf);
        return;
    }

    // zona/N/pompa
    char prefix[64];
    snprintf(prefix, sizeof(prefix), "nestgrow/%s/zona/", s_device);
    if (strncmp(raw_topic, prefix, strlen(prefix)) == 0) {
        int zona = atoi(raw_topic + strlen(prefix));
        if (zona < 1 || zona > 4) return;
        JsonDocument doc;
        if (deserializeJson(doc, buf)) return;
        const char *cmd = doc["cmd"] | "";
        if (strcmp(cmd, "on") == 0) {
            int sec = doc["sec"] | 30;
            valves_open(zona, sec);
            Serial.printf("[MQTT] Zona %d ON per %d sec\n", zona, sec);
        } else if (strcmp(cmd, "off") == 0) {
            valves_close(zona);
            Serial.printf("[MQTT] Zona %d OFF\n", zona);
        }
    }
}

// ── connect / subscribe ───────────────────────────────────────────────────────

static bool do_connect() {
    if (WiFi.status() != WL_CONNECTED) return false;
    if (!mc.connect(s_device)) {
        Serial.printf("[MQTT] Connect failed rc=%d\n", mc.state());
        return false;
    }
    char t[80];
    for (int i = 1; i <= 4; i++) {
        char suf[32];
        snprintf(suf, sizeof(suf), "zona/%d/pompa", i);
        topic(t, sizeof(t), suf);
        mc.subscribe(t);
    }
    topic(t, sizeof(t), "cmd/reconfig");  mc.subscribe(t);
    topic(t, sizeof(t), "cmd/config");    mc.subscribe(t);
    Serial.printf("[MQTT] Connected as %s\n", s_device);
    return true;
}

// ── public API ────────────────────────────────────────────────────────────────

void mqtt_init(const NVSConfig &cfg) {
    strlcpy(s_device, cfg.device_name, sizeof(s_device));
    strlcpy(s_host,   cfg.mqtt_host,   sizeof(s_host));
    s_port = cfg.mqtt_port;
    for (int i = 0; i < 4; i++) s_zone_interval[i] = cfg.zona_interval[i];

    mc.setServer(s_host, s_port);
    mc.setCallback(on_message);
    mc.setKeepAlive(60);
    mc.setBufferSize(512);

    do_connect();
}

void mqtt_loop() {
    if (mc.connected()) { mc.loop(); return; }
    unsigned long now = millis();
    if (now - s_last_reconnect >= RECONNECT_MS) {
        s_last_reconnect = now;
        do_connect();
    }
}

bool mqtt_is_connected() {
    return mc.connected();
}

int mqtt_get_zone_interval(int zona) {
    if (zona < 1 || zona > 4) return (int)DEFAULT_INTERVAL_MS;
    return s_zone_interval[zona - 1];
}

void mqtt_publish_soil(int zona, float pct) {
    if (!mc.connected()) return;
    char suf[32], t[80], payload[128];
    snprintf(suf, sizeof(suf), "zona/%d/umidita", zona);
    topic(t, sizeof(t), suf);
    snprintf(payload, sizeof(payload),
        "{\"v\":%.1f,\"ts\":%lu,\"device_id\":\"%s\"}",
        pct, getTimestamp(), s_device);
    mc.publish(t, payload);
}

void mqtt_publish_tank(float pct) {
    if (!mc.connected()) return;
    char t[80], payload[64];
    topic(t, sizeof(t), "serbatoio/livello");
    snprintf(payload, sizeof(payload),
        "{\"v\":%.1f,\"ts\":%lu}", pct, getTimestamp());
    mc.publish(t, payload);
}

void mqtt_publish_heartbeat() {
    if (!mc.connected()) return;
    char t[80], payload[384];
    topic(t, sizeof(t), "heartbeat");

    String ora = ntp_local_time_iso();

    snprintf(payload, sizeof(payload),
        "{\"uptime_sec\":%lu,\"wifi_rssi\":%d,\"free_heap\":%u,"
        "\"firmware_version\":\"%s\",\"ip\":\"%s\",\"device_id\":\"%s\","
        "\"ntp_sync\":%s,\"ora_locale\":\"%s\","
        "\"zona_intervals\":[%d,%d,%d,%d]}",
        millis() / 1000UL,
        (int)WiFi.RSSI(),
        (unsigned int)ESP.getFreeHeap(),
        FIRMWARE_VERSION,
        WiFi.localIP().toString().c_str(),
        s_device,
        ntp_is_synced() ? "true" : "false",
        ora.c_str(),
        s_zone_interval[0], s_zone_interval[1],
        s_zone_interval[2], s_zone_interval[3]);

    mc.publish(t, payload);
}

void mqtt_publish_valve_state(int zona, bool is_open, int sec_remaining) {
    if (!mc.connected()) return;
    char suf[32], t[80], payload[64];
    snprintf(suf, sizeof(suf), "zona/%d/pompa/stato", zona);
    topic(t, sizeof(t), suf);
    if (is_open) {
        snprintf(payload, sizeof(payload),
            "{\"state\":\"on\",\"sec_remaining\":%d}", sec_remaining);
    } else {
        strlcpy(payload, "{\"state\":\"off\"}", sizeof(payload));
    }
    mc.publish(t, payload);
}
