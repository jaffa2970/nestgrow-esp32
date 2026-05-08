#include "ntp.h"
#include <time.h>

void syncNTP() {
    // UTC+1 base, 3600s DST (ora legale italiana)
    configTime(3600, 3600, "pool.ntp.org", "time.nist.gov");
    Serial.print("[NTP] Sincronizzazione");
    struct tm timeinfo;
    int retry = 0;
    while (!getLocalTime(&timeinfo) && retry < 20) {
        Serial.print(".");
        delay(500);
        retry++;
    }
    if (retry < 20) {
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
        Serial.println(" OK");
        Serial.printf("[NTP] Ora: %s\n", buf);
    } else {
        Serial.println(" FALLITO - uso uptime");
    }
}

unsigned long getTimestamp() {
    struct tm t;
    if (!getLocalTime(&t)) return millis() / 1000UL;
    time_t epoch = mktime(&t);
    return (unsigned long)epoch;
}

bool ntp_is_synced() {
    struct tm t;
    if (!getLocalTime(&t)) return false;
    return (t.tm_year + 1900) >= 2020;
}

String ntp_local_time_iso() {
    struct tm t;
    if (!getLocalTime(&t)) return String("");
    char buf[24];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &t);
    return String(buf);
}
