#pragma once
#include <Arduino.h>

void valves_init();
void valves_open(int zona, int seconds);
void valves_close(int zona);
void valves_safety_check(float tank_pct);
bool valves_is_open(int zona);
int  valves_seconds_remaining(int zona);
