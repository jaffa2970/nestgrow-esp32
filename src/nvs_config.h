#pragma once
#include <Arduino.h>

struct NVSConfig {
    char ssid[64];
    char password[64];
    char mqtt_host[64];
    int  mqtt_port;
    char device_name[32];
    bool use_dhcp;
    char static_ip[16];
    char gateway[16];
    char subnet[16];
    bool configured;
    int  zona_interval[4];  // per-zone publish interval in ms (default DEFAULT_INTERVAL_MS)
};

void nvs_load(NVSConfig &cfg);
void nvs_save(const NVSConfig &cfg);
void nvs_clear();
void nvs_generate_device_name(char *out, size_t len);
