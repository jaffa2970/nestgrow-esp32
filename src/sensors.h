#pragma once
#include <Arduino.h>

void  sensors_init();
float sensors_read_soil(int zona);  // zona 1-4, returns 0.0-100.0 %
float sensors_read_float();          // 100.0 = full, 0.0 = empty
