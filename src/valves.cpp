#include "valves.h"
#include "config.h"

static const int RELAY_PINS[4] = {RELAY_1, RELAY_2, RELAY_3, RELAY_4};

static bool          s_open[4]     = {false, false, false, false};
static unsigned long s_open_ms[4]  = {0, 0, 0, 0};
static unsigned long s_dur_ms[4]   = {0, 0, 0, 0};

void valves_init() {
    for (int i = 0; i < 4; i++) {
        pinMode(RELAY_PINS[i], OUTPUT);
        digitalWrite(RELAY_PINS[i], HIGH);  // HIGH = OFF (inverted logic)
    }
}

void valves_open(int zona, int seconds) {
    if (zona < 1 || zona > 4) return;
    int z = zona - 1;
    digitalWrite(RELAY_PINS[z], LOW);       // LOW = ON
    s_open[z]    = true;
    s_open_ms[z] = millis();
    s_dur_ms[z]  = (unsigned long)seconds * 1000UL;
}

void valves_close(int zona) {
    if (zona < 1 || zona > 4) return;
    int z = zona - 1;
    digitalWrite(RELAY_PINS[z], HIGH);      // HIGH = OFF
    s_open[z]    = false;
    s_open_ms[z] = 0;
    s_dur_ms[z]  = 0;
}

void valves_safety_check(float tank_pct) {
    unsigned long now = millis();

    if (tank_pct <= 0.0f) {
        for (int i = 1; i <= 4; i++) {
            if (s_open[i - 1]) {
                valves_close(i);
                Serial.printf("[SAFETY] Serbatoio vuoto — chiusura zona %d\n", i);
            }
        }
        return;
    }

    for (int i = 1; i <= 4; i++) {
        int z = i - 1;
        if (!s_open[z]) continue;

        if (now - s_open_ms[z] >= VALVE_TIMEOUT_MS) {
            valves_close(i);
            Serial.printf("[SAFETY] Timeout — chiusura zona %d\n", i);
        } else if (now - s_open_ms[z] >= s_dur_ms[z]) {
            valves_close(i);
        }
    }
}

bool valves_is_open(int zona) {
    if (zona < 1 || zona > 4) return false;
    return s_open[zona - 1];
}

int valves_seconds_remaining(int zona) {
    if (zona < 1 || zona > 4) return 0;
    int z = zona - 1;
    if (!s_open[z]) return 0;
    unsigned long elapsed = millis() - s_open_ms[z];
    if (elapsed >= s_dur_ms[z]) return 0;
    return (int)((s_dur_ms[z] - elapsed) / 1000UL);
}
