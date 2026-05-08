#include "sensors.h"
#include "config.h"

static const int SOIL_PINS[4] = {SOIL_1, SOIL_2, SOIL_3, SOIL_4};
static const int N_SAMPLES = 5;

static int  s_buf[4][N_SAMPLES];
static int  s_idx[4]    = {0, 0, 0, 0};
static bool s_full[4]   = {false, false, false, false};

void sensors_init() {
    for (int i = 0; i < 4; i++) {
        pinMode(SOIL_PINS[i], INPUT);
        // Pre-fill buffers so first reading is not zero
        for (int s = 0; s < N_SAMPLES; s++) {
            s_buf[i][s] = analogRead(SOIL_PINS[i]);
        }
        s_full[i] = true;
    }
    pinMode(FLOAT_SW, INPUT);
}

float sensors_read_soil(int zona) {
#ifdef DEBUG_SENSORS
    return (float)random(20, 80);
#endif
    if (zona < 1 || zona > 4) return 0.0f;
    int z = zona - 1;

    s_buf[z][s_idx[z]] = analogRead(SOIL_PINS[z]);
    s_idx[z] = (s_idx[z] + 1) % N_SAMPLES;
    if (s_idx[z] == 0) s_full[z] = true;

    int count = s_full[z] ? N_SAMPLES : (s_idx[z] == 0 ? 1 : s_idx[z]);
    long sum = 0;
    for (int i = 0; i < count; i++) sum += s_buf[z][i];
    int avg = (int)(sum / count);

    float pct = (float)(SOIL_DRY - avg) / (float)(SOIL_DRY - SOIL_WET) * 100.0f;
    if (pct < 0.0f)   pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;
    return pct;
}

float sensors_read_float() {
#ifdef DEBUG_SENSORS
    return 100.0f;
#endif
    return (digitalRead(FLOAT_SW) == HIGH) ? 100.0f : 0.0f;
}
