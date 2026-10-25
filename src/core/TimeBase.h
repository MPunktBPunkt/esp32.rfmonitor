#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

// Wall-clock vs uptime. Until NTP succeeds, exports carry timeValid=false.
namespace TimeBase {
void begin();
void onWifiConnected();
void loop();

uint32_t uptimeS();
bool timeValid();
uint32_t epochS();  // 0 if !timeValid()
void appendJson(JsonObject target);
}
