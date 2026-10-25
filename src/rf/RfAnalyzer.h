#pragma once

#include <WiFi.h>
#include "BuildFlags.h"
#include "core/ConfigStore.h"
#include "core/EventLog.h"
#include "core/HistoryStore.h"
#include "rf/RfScanner.h"
#include "rf/BleTracker.h"
#include "rf/LocationProfile.h"
#include "rf/MeasSession.h"
#include "rf/RfScore.h"

class RfAnalyzer {
public:
    void begin(ConfigStore* cfg, EventLog* events, HistoryStore* history, RfScanner* scanner, BleTracker* ble);
    void loop();
    void noteHub(bool ok);

    uint8_t score() const { return score_; }
    uint8_t scoreV1() const { return scoreV1_; }
    uint8_t scoreV2() const { return scoreV2_; }
    uint8_t scoreVersion() const { return RFMON_SCORE_VERSION; }
    uint8_t measurementQuality() const { return quality_; }
    int8_t locationDeviation() const { return location_.deviation(); }
    float rssiAvg() const { return rssiAvg_; }
    float rssiJitter() const { return rssiJitter_; }
    int8_t rssiMin() const { return rssiMin_; }
    int8_t rssiMax() const { return rssiMax_; }
    uint8_t congestion() const { return congestion_; }
    uint8_t channelLoad() const { return channelLoad_; }
    uint8_t density() const { return density_; }
    uint8_t stability() const { return stability_; }
    uint16_t disconnects() const { return disconnects_; }
    float radioDutyPct() const { return radioDutyPct_; }           // lifetime (compat)
    float radioDuty5mPct() const { return radioDuty5mPct_; }
    float radioDuty1hPct() const { return radioDuty1hPct_; }
    LocationProfile& location() { return location_; }
    const LocationProfile& location() const { return location_; }
    MeasSession& session() { return session_; }
    const MeasSession& session() const { return session_; }

    void appendStatusJson(JsonObject target) const;
    void appendObservationJson(JsonObject target) const;
    void appendSnapshotJson(JsonObject target) const;
    void appendFindingsJson(JsonArray arr) const;
    void appendAlertState(JsonObject target) const;
    void appendHubContract(JsonObject target) const;
    void appendIoValues(JsonObject ios) const;
    void appendWhyJson(JsonObject target) const;
    bool anyAlert() const;
    bool ackAlert(const char* code);
    bool ackAllAlerts();

private:
    enum class CondState : uint8_t { Inactive = 0, Pending = 1, Active = 2, Acked = 3, Recovering = 4 };
    struct Hyst {
        CondState state = CondState::Inactive;
        bool active = false;
        bool acked = false;
        unsigned long enterSince = 0;
        unsigned long clearSince = 0;
        unsigned long lastEmit = 0;
        unsigned long activeSince = 0;
        const char* code = "";
    };
    void recompute();
    void recomputeQuality();
    void updateAlerts();
    void updateRadioDuty();
    bool tickHyst(Hyst& h, bool enterCond, bool exitCond, unsigned long enterMs, unsigned long exitMs,
                  unsigned long cooldownMs, const char* code, const char* enterType, const char* enterTitle,
                  const char* exitType, const char* exitTitle, const char* detail);
    void putAlertCond(JsonObject parent, const char* key, const Hyst& h) const;
    static const char* condName(CondState s);
    static void putProxy(JsonObject parent, const char* key, uint8_t value, const char* basis);
    static void putParts(JsonObject parts, const RfScoreBreakdown& b);
    Hyst* findAlert(const char* code);

    ConfigStore* cfg_ = nullptr;
    EventLog* events_ = nullptr;
    HistoryStore* history_ = nullptr;
    RfScanner* scanner_ = nullptr;
    BleTracker* ble_ = nullptr;
    LocationProfile location_;
    MeasSession session_;
    uint8_t score_ = 0;
    uint8_t scoreV1_ = 0;
    uint8_t scoreV2_ = 0;
    uint8_t quality_ = 0;
    float rssiAvg_ = 0;
    float rssiJitter_ = 0;
    float radioDutyPct_ = 0;      // lifetime since boot
    float radioDuty5mPct_ = 0;
    float radioDuty1hPct_ = 0;
    uint32_t lastBusyMs_ = 0;
    uint32_t busyThisMin_ = 0;
    unsigned long minuteMarkMs_ = 0;
    uint32_t busyPerMin_[60] = {0};
    uint8_t busyMinHead_ = 0;
    uint8_t busyMinFilled_ = 0;
    int8_t rssiMin_ = 0;
    int8_t rssiMax_ = 0;
    uint8_t congestion_ = 0;
    uint8_t channelLoad_ = 0;
    uint8_t density_ = 0;
    uint8_t stability_ = 100;
    RfScoreBreakdown partsV1_{};
    RfScoreBreakdown partsV2_{};
    // Legacy flat penalty fields (mirror active score parts for old UI).
    uint8_t penRssi_ = 0;
    uint8_t penJitter_ = 0;
    uint8_t penCongestion_ = 0;
    uint8_t penDensity_ = 0;
    uint8_t penStability_ = 0;
    uint8_t penDisconnect_ = 0;
    unsigned long lastCompute_ = 0;
    int8_t rssiWindow_[12] = {0};
    uint8_t rssiCount_ = 0;
    uint8_t rssiHead_ = 0;
    uint16_t disconnects_ = 0;
    wl_status_t lastWifi_ = WL_IDLE_STATUS;
    int8_t lastRssi_ = 0;
    bool hubWasOk_ = true;
    Hyst hystScore_;
    Hyst hystPressure_;
    Hyst hystQuality_;
    Hyst hystDuty_;
    Hyst hystDeviation_;
};
