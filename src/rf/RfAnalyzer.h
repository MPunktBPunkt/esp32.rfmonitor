#pragma once

#include <WiFi.h>
#include "core/ConfigStore.h"
#include "core/EventLog.h"
#include "core/HistoryStore.h"
#include "rf/RfScanner.h"
#include "rf/BleTracker.h"

class RfAnalyzer {
public:
    void begin(ConfigStore* cfg, EventLog* events, HistoryStore* history, RfScanner* scanner, BleTracker* ble);
    void loop();
    void noteHub(bool ok);

    uint8_t score() const { return score_; }
    float rssiAvg() const { return rssiAvg_; }
    float rssiJitter() const { return rssiJitter_; }
    int8_t rssiMin() const { return rssiMin_; }
    int8_t rssiMax() const { return rssiMax_; }
    uint8_t congestion() const { return congestion_; }
    uint8_t channelLoad() const { return channelLoad_; }
    uint8_t density() const { return density_; }
    uint8_t stability() const { return stability_; }
    uint16_t disconnects() const { return disconnects_; }
    void appendStatusJson(JsonObject target) const;
    void appendIoValues(JsonObject ios) const;

private:
    void recompute();

    ConfigStore* cfg_ = nullptr;
    EventLog* events_ = nullptr;
    HistoryStore* history_ = nullptr;
    RfScanner* scanner_ = nullptr;
    BleTracker* ble_ = nullptr;
    uint8_t score_ = 0;
    float rssiAvg_ = 0;
    float rssiJitter_ = 0;
    int8_t rssiMin_ = 0;
    int8_t rssiMax_ = 0;
    uint8_t congestion_ = 0;
    uint8_t channelLoad_ = 0;
    uint8_t density_ = 0;
    uint8_t stability_ = 100;
    unsigned long lastCompute_ = 0;
    int8_t rssiWindow_[12] = {0};
    uint8_t rssiCount_ = 0;
    uint8_t rssiHead_ = 0;
    uint16_t disconnects_ = 0;
    wl_status_t lastWifi_ = WL_IDLE_STATUS;
    int8_t lastRssi_ = 0;
    bool hubWasOk_ = true;
};
