#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "BuildFlags.h"

class ConfigStore {
public:
    String deviceName = DEVICE_NAME_DEFAULT;
    String locationName = "Standort";
    String hubHost = HUB_HOST_DEFAULT;
    int hubPort = HUB_PORT_DEFAULT;
    bool enableHub = true;
    uint16_t heartbeatIntervalS = 30;
    uint8_t logLevel = 2;
    bool enableMdns = true;
    uint16_t watchdogS = 300;

    uint16_t wifiScanIntervalS = 20;
    uint16_t bleScanIntervalS = 15;
    bool bleEnabled = true;
    String knownSsids;
    String knownBle;
    int8_t rssiWarnDrop = 12;
    uint8_t rfScoreWarn = 40;
    uint8_t congestionWarn = 70;
    String opMode = "normal";  // live | normal | diagnostic
    bool firstRunDone = false;

    void begin();
    void load();
    void save();
    void factoryReset();
    void applyDefaults();
    void applyModeIntervals();
    void toJson(JsonObject obj) const;
    bool fromJson(JsonVariantConst obj);

private:
    static constexpr uint8_t kConfigVersion = 1;
};
