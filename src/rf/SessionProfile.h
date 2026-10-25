#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

// Named, reproducible measurement campaigns (Field / Characterization / Motion / …).
struct SessionProfileDef {
    const char* id;
    const char* title;
    uint16_t durationS;          // 0 = manual stop
    uint16_t wifiScanIntervalS;
    uint16_t bleScanIntervalS;
    bool bleEnabled;
    bool characterization;       // enables phase UI / markers (A–G or R0–R4)
    bool motionSensitivity;      // high-rate own-link RSSI capture
    uint16_t motionRssiIntervalMs; // 0 = unused; typical 200
};

namespace SessionProfile {
const SessionProfileDef* find(const char* id);
void listToJson(JsonArray arr);
const SessionProfileDef& field();
const SessionProfileDef& characterization();
const SessionProfileDef& motion();
}
