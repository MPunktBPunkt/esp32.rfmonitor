#pragma once

#include "core/ConfigStore.h"
#include "core/EventLog.h"
#include "core/HistoryStore.h"
#include "BuildFlags.h"

struct ApRecord {
    char ssid[33];
    char bssid[18];
    int8_t rssi;
    uint8_t channel;
    uint8_t enc;
    bool hidden;
    bool known;
    uint32_t lastSeen;
};

class RfScanner {
public:
    void begin(ConfigStore* cfg, EventLog* events, HistoryStore* history);
    void loop(bool mayStart = true);
    bool due() const;
    bool requestScan();
    bool busy() const { return scanning_; }
    void pause();

    uint8_t apCount() const { return apCount_; }
    uint8_t knownCount() const { return knownCount_; }
    uint8_t unknownCount() const { return apCount_ > knownCount_ ? apCount_ - knownCount_ : 0; }
    int8_t strongestRssi() const { return strongest_; }
    float averageRssi() const { return avgRssi_; }
    uint16_t failCount() const { return failCount_; }
    uint32_t lastScanTs() const { return lastScanTs_; }
    const ApRecord* aps() const { return aps_; }
    void channelOccupancy(uint8_t out[14]) const;
    void resultsToJson(JsonArray arr) const;
    void channelsToJson(JsonArray arr) const;

private:
    void processScan(int16_t n);
    bool isKnownSsid(const String& ssid) const;
    void emitDiff();

    ConfigStore* cfg_ = nullptr;
    EventLog* events_ = nullptr;
    HistoryStore* history_ = nullptr;
    ApRecord aps_[RFMON_MAX_APS];
    char prevBssid_[RFMON_MAX_APS][18];
    uint8_t prevCount_ = 0;
    uint8_t apCount_ = 0;
    uint8_t knownCount_ = 0;
    int8_t strongest_ = -127;
    float avgRssi_ = 0;
    bool scanning_ = false;
    unsigned long lastScanStart_ = 0;
    uint32_t lastScanTs_ = 0;
    uint16_t failCount_ = 0;
};
