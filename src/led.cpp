#include "led.h"
#include "config.h"

static LedState s_state = LedState::AP_MODE;

void led_init() {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
}

void led_set_state(LedState state) {
    s_state = state;
}

LedState led_get_state() {
    return s_state;
}

void led_update() {
    unsigned long t = millis();

    switch (s_state) {
        case LedState::AP_MODE:
            // 200ms on/off
            digitalWrite(LED_PIN, (t / 200UL) % 2 ? HIGH : LOW);
            break;

        case LedState::CONNECTING:
            // 1000ms on/off
            digitalWrite(LED_PIN, (t / 1000UL) % 2 ? HIGH : LOW);
            break;

        case LedState::CONNECTED:
            digitalWrite(LED_PIN, HIGH);
            break;

        case LedState::ERROR: {
            // 3 fast blinks (150ms on/off each) then ~1550ms off — 2s total
            unsigned long phase = t % 2000UL;
            bool on = (phase < 150) ||
                      (phase >= 300 && phase < 450) ||
                      (phase >= 600 && phase < 750);
            digitalWrite(LED_PIN, on ? HIGH : LOW);
            break;
        }
    }
}
