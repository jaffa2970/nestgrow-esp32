#pragma once
#include <Arduino.h>

void          syncNTP();
unsigned long getTimestamp();   // Unix epoch, fallback su uptime
bool          ntp_is_synced();
String        ntp_local_time_iso();  // "2026-05-08T19:32:13"
