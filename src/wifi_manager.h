#pragma once
#include <Arduino.h>
#include "nvs_config.h"

void wifi_start_ap(const NVSConfig &cfg);
bool wifi_connect_station(const NVSConfig &cfg);
void wifi_loop();          // call every loop() — handles DNS in AP mode and restart flag
bool wifi_is_ap_mode();
