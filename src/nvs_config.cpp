#include "nvs_config.h"
#include "config.h"
#include <Preferences.h>
#include <WiFi.h>

static Preferences prefs;

void nvs_generate_device_name(char *out, size_t len) {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(out, len, "nestgrow-%02x%02x", mac[4], mac[5]);
}

void nvs_load(NVSConfig &cfg) {
    memset(&cfg, 0, sizeof(cfg));
    cfg.mqtt_port  = MQTT_PORT;
    cfg.use_dhcp   = true;
    cfg.configured = false;
    for (int i = 0; i < 4; i++) cfg.zona_interval[i] = DEFAULT_INTERVAL_MS;

    nvs_generate_device_name(cfg.device_name, sizeof(cfg.device_name));

    prefs.begin("nestgrow", true);  // read-only

    if (!prefs.isKey("configured")) {
        prefs.end();
        return;
    }

    cfg.configured = prefs.getBool("configured", false);
    prefs.getString("ssid",        cfg.ssid,        sizeof(cfg.ssid));
    prefs.getString("password",    cfg.password,    sizeof(cfg.password));
    prefs.getString("mqtt_host",   cfg.mqtt_host,   sizeof(cfg.mqtt_host));
    cfg.mqtt_port  = prefs.getInt("mqtt_port", MQTT_PORT);
    prefs.getString("device_name", cfg.device_name, sizeof(cfg.device_name));
    cfg.use_dhcp   = prefs.getBool("use_dhcp", true);
    prefs.getString("static_ip",   cfg.static_ip,   sizeof(cfg.static_ip));
    prefs.getString("gateway",     cfg.gateway,     sizeof(cfg.gateway));
    prefs.getString("subnet",      cfg.subnet,      sizeof(cfg.subnet));
    for (int i = 0; i < 4; i++) {
        char k[8];
        snprintf(k, sizeof(k), "zint%d", i);
        cfg.zona_interval[i] = prefs.getInt(k, (int)DEFAULT_INTERVAL_MS);
    }

    prefs.end();
}

void nvs_save(const NVSConfig &cfg) {
    prefs.begin("nestgrow", false);  // read-write
    prefs.putBool(  "configured",  cfg.configured);
    prefs.putString("ssid",        cfg.ssid);
    prefs.putString("password",    cfg.password);
    prefs.putString("mqtt_host",   cfg.mqtt_host);
    prefs.putInt(   "mqtt_port",   cfg.mqtt_port);
    prefs.putString("device_name", cfg.device_name);
    prefs.putBool(  "use_dhcp",    cfg.use_dhcp);
    prefs.putString("static_ip",   cfg.static_ip);
    prefs.putString("gateway",     cfg.gateway);
    prefs.putString("subnet",      cfg.subnet);
    for (int i = 0; i < 4; i++) {
        char k[8];
        snprintf(k, sizeof(k), "zint%d", i);
        prefs.putInt(k, cfg.zona_interval[i]);
    }
    prefs.end();
}

void nvs_clear() {
    prefs.begin("nestgrow", false);
    prefs.clear();
    prefs.end();
}
