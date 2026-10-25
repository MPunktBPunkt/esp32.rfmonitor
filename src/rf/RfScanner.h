#pragma once

#include "core/ConfigStore.h"
#include "core/EventLog.h"
#include "core/HistoryStore.h"
#include "BuildFlags.h"

enum class ApLife : uint8_t {
    New = 0,
    Persistent = 1,
    Flapping = 2,
    Disappeared = 3
};

struct ApRecord {
    char ssid[33];
    char bssid[18];
    int8_t rssi;
    uint8_t channel;
    uint8_t enc;
    bool hidden;
    bool known;
    uint32_t firstSeen;
    uint32_t lastSeen;
    uint16_t seenCount;
    uint8_t missStreak;
    uint8_t flaps;
    ApLife life;
};

class RfScanner {
public:
    void begin(ConfigStore* cfg, EventLog* events, HistoryStore* history);
    void loop(bool mayStart = true);
    bool due() const;
    bool requestScan();
    bool busy() const { return scanning_; }
    void pause();

    uint8_t apCount() const { return apCount_; }           // currently visible (not disappeared)
    uint8_t trackedCount() const { return tracked_; }      // incl. recently disappeared
    uint8_t knownCount() const { return knownCount_; }
    uint8_t unknownCount() const { return apCount_ > knownCount_ ? apCount_ - knownCount_ : 0; }
    uint8_t newCount() const { return lifeNew_; }
    uint8_t persistentCount() const { return lifePersistent_; }
    uint8_t flappingCount() const { return lifeFlapping_; }
    uint8_t disappearedCount() const { return lifeDisappeared_; }
    int8_t strongestRssi() const { return strongest_; }
    float averageRssi() const { return avgRssi_; }
    uint16_t failCount() const { return failCount_; }
    uint16_t attemptCount() const { return attemptCount_; }
    uint16_t successCount() const { return successCount_; }
    uint32_t lastScanTs() const { return lastScanTs_; }
    uint32_t lastScanDurationMs() const { return lastDurationMs_; }
    uint32_t totalBusyMs() const { return totalBusyMs_; }
    bool lastScanOk() const { return lastScanOk_; }
    uint32_t scanAgeS() const;
    float failRatePct() const;
    size_t capacity() const { return capacity_; }
    const ApRecord* aps() const { return aps_; }
    void channelOccupancy(uint8_t out[14]) const;
    void channelStats(uint8_t ch, uint8_t& count, int8_t& maxRssi, int8_t& avgRssi,
                      uint8_t& known, uint8_t& unknown) const;
    void resultsToJson(JsonArray arr) const;
    void channelsToJson(JsonArray arr) const;
    void appendLifeJson(JsonObject target) const;
    static const char* lifeName(ApLife life);

private:
    void processScan(int16_t n);
    bool isKnownSsid(const String& ssid) const;
    void emitDiff(uint8_t newIdx, bool disappeared);
    void noteFail();
    int findIndex(const char* bssid) const;
    int allocSlot();
    void recomputeLife();
    void prune(uint32_t nowS);

    ConfigStore* cfg_ = nullptr;
    EventLog* events_ = nullptr;
    HistoryStore* history_ = nullptr;
    ApRecord* aps_ = nullptr;
    size_t capacity_ = RFMON_MAX_APS;
    uint8_t tracked_ = 0;
    uint8_t apCount_ = 0;
    uint8_t knownCount_ = 0;
    uint8_t lifeNew_ = 0;
    uint8_t lifePersistent_ = 0;
    uint8_t lifeFlapping_ = 0;
    uint8_t lifeDisappeared_ = 0;
    int8_t strongest_ = -127;
    float avgRssi_ = 0;
    bool scanning_ = false;
    bool lastScanOk_ = false;
    unsigned long lastScanStart_ = 0;
    uint32_t lastScanTs_ = 0;
    uint32_t lastDurationMs_ = 0;
    uint32_t totalBusyMs_ = 0;
    uint16_t failCount_ = 0;
    uint16_t attemptCount_ = 0;
    uint16_t successCount_ = 0;
};
