#pragma once
#include <Arduino.h>

enum class LedState {
    AP_MODE,     // fast blink 200ms on/off
    CONNECTING,  // slow blink 1000ms on/off
    CONNECTED,   // solid on
    ERROR        // 3 fast blinks every 2s
};

void     led_init();
void     led_set_state(LedState state);
LedState led_get_state();
void     led_update();  // call every loop()
