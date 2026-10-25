#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

// Named, reproducible measurement campaigns (Field / Characterization / …).
struct SessionProfileDef {
    const char* id;
    const char* title;
    uint16_t durationS;          // 0 = manual stop
    uint16_t wifiScanIntervalS;
    uint16_t bleScanIntervalS;
    bool bleEnabled;
    bool characterization;       // enables phase UI / markers
};

namespace SessionProfile {
const SessionProfileDef* find(const char* id);
void listToJson(JsonArray arr);
const SessionProfileDef& field();
const SessionProfileDef& characterization();
}
