#pragma once

#include "core/ConfigStore.h"
#include "core/EventLog.h"
#include "core/HistoryStore.h"
#include "BuildFlags.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

struct BleRecord {
    char addr[18];
    char name[32];
    int8_t rssi;
    uint16_t mfgId;
    uint8_t kind;       // 0=advertiser 1=ibeacon 2=eddystone 3=generic-beacon
    uint16_t hits;
    uint32_t firstSeen;
    uint32_t lastSeen;
    bool known;
};

class BleTracker {
public:
    void begin(ConfigStore* cfg, EventLog* events, HistoryStore* history);
    void loop(bool mayStart = true);
    bool due() const;
    bool startWindow();
    void stop();
    bool busy() const { return scanning_; }
    bool ready() const { return ready_; }

    uint8_t deviceCount() const { return count_; }
    uint8_t beaconCount() const { return beaconCount_; }
    uint8_t newCount() const { return newCount_; }
    uint16_t failCount() const { return failCount_; }
    uint16_t attemptCount() const { return attemptCount_; }
    uint16_t successCount() const { return successCount_; }
    uint32_t lastScanDurationMs() const { return lastDurationMs_; }
    uint32_t totalBusyMs() const { return totalBusyMs_; }
    uint32_t lastScanTs() const { return lastScanTs_; }
    uint32_t scanAgeS() const;
    float failRatePct() const;
    size_t capacity() const { return capacity_; }
    void resultsToJson(JsonArray arr, bool beaconsOnly = false) const;
    void ingest(const char* addr, const char* name, int8_t rssi, uint16_t mfgId, uint8_t kind);

    // Soft hang recovery: active scan window is 4s; force-finish after this.
    static constexpr uint32_t kWindowTimeoutMs = 7000UL;

private:
    int findIndex(const char* addr) const;
    bool isKnown(const char* addr, const char* name) const;
    void prune();
    void noteFail();
    void finishWindow(bool ok);

    ConfigStore* cfg_ = nullptr;
    EventLog* events_ = nullptr;
    HistoryStore* history_ = nullptr;
    BleRecord* devices_ = nullptr;
    size_t capacity_ = RFMON_MAX_BLE;
    uint8_t count_ = 0;
    uint8_t beaconCount_ = 0;
    uint8_t newCount_ = 0;
    bool scanning_ = false;
    bool ready_ = false;
    unsigned long windowStart_ = 0;
    unsigned long lastStart_ = 0;
    unsigned long lastPrune_ = 0;
    uint32_t lastDurationMs_ = 0;
    uint32_t totalBusyMs_ = 0;
    uint32_t lastScanTs_ = 0;
    uint16_t failCount_ = 0;
    uint16_t attemptCount_ = 0;
    uint16_t successCount_ = 0;
    char pendingAddr_[8][18];
    uint8_t pendingKind_[8];
    uint8_t pendingCount_ = 0;
    portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;
};
