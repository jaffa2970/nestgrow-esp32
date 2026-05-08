#pragma once
#include <Arduino.h>
#include "nvs_config.h"

void mqtt_init(const NVSConfig &cfg);
void mqtt_loop();
bool mqtt_is_connected();
void mqtt_publish_soil(int zona, float pct);       // single-zone soil publish
void mqtt_publish_tank(float pct);                 // tank level publish
void mqtt_publish_heartbeat();
void mqtt_publish_valve_state(int zona, bool is_open, int sec_remaining);
int  mqtt_get_zone_interval(int zona);             // zona 1-4, returns ms
