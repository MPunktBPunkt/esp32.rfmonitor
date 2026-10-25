#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "BuildFlags.h"
#include "rf/SessionProfile.h"

// Reproducible field measurement window (summary + ring of samples + phases).
// Per-phase running aggregates survive ring wrap so A–G exports stay complete.
class MeasSession {
public:
#if defined(RFMON_BOARD_S3)
    static constexpr size_t kMaxSamples = 300;
#else
    static constexpr size_t kMaxSamples = 180;
#endif
    static constexpr size_t kMaxPhases = 12;

    void begin();
    bool active() const { return active_; }
    bool start(const char* label = "", const SessionProfileDef* profile = nullptr,
               uint16_t durationOverrideS = 0);
    bool stop();
    bool markPhase(const char* id, const char* note = "");
    // Call from App loop — auto-stops when duration elapsed.
    bool loopAutoStop();

    const char* profileId() const { return profileId_; }
    uint16_t durationS() const { return durationS_; }
    const char* currentPhase() const { return currentPhaseId_; }

    void noteSample(uint8_t score, uint8_t quality, int8_t deviation,
                    uint8_t apCount, uint8_t bleCount, uint8_t pressure,
                    uint8_t occupancy, uint8_t density, float jitter, uint8_t stability,
                    bool connected, float ownRssi, float radioDuty, uint8_t eventLevel);
    void appendStatusJson(JsonObject target) const;
    void appendExportJson(JsonObject target) const;

private:
    struct Sample {
        uint32_t tsUptime;
        uint32_t tsEpoch;
        uint8_t score;
        uint8_t quality;
        int8_t deviation;
        uint8_t apCount;
        uint8_t bleCount;
        uint8_t pressure;
        uint8_t occupancy;
        uint8_t density;
        uint8_t jitter;
        uint8_t stability;
        uint8_t flags;      // bit0 = connected
        int8_t ownRssi;
        uint8_t radioDuty;
        char phase[4];
    };
    struct Phase {
        char id[4];
        char note[40];
        uint32_t startSample;
        uint32_t endSample;   // 0 while open
        uint32_t startUptimeS;
        uint32_t endUptimeS;
        bool open;
        // Running aggregates (survive sample ring wrap).
        uint32_t n;
        uint32_t connectedN;
        double sumScore;
        double sumQuality;
        double sumDev;
        double sumAp;
        double sumBle;
        double sumPress;
        double sumOcc;
        double sumDens;
        double sumJitter;
        double sumStab;
        double sumRssi;
        double sumDuty;
        uint8_t minScore;
        uint8_t maxScore;
    };

    void closeOpenPhase(uint32_t sampleIdx, uint32_t uptimeS);
    void accumulatePhase(Phase& p, uint8_t score, uint8_t quality, int8_t deviation,
                         uint8_t apCount, uint8_t bleCount, uint8_t pressure,
                         uint8_t occupancy, uint8_t density, float jitter, uint8_t stability,
                         bool connected, float ownRssi, float radioDuty);
    void appendPhaseJson(JsonObject o, const Phase& p) const;

    bool active_ = false;
    char label_[40] = {0};
    char profileId_[20] = {0};
    char profileTitle_[28] = {0};
    uint16_t durationS_ = 0;
    uint16_t wifiIntervalS_ = 0;
    uint16_t bleIntervalS_ = 0;
    bool bleEnabled_ = true;
    bool characterization_ = false;
    char currentPhaseId_[4] = {0};

    uint32_t startS_ = 0;
    uint32_t endS_ = 0;
    uint32_t startEpochS_ = 0;
    uint32_t endEpochS_ = 0;
    bool startTimeValid_ = false;
    bool endTimeValid_ = false;
    uint32_t sampleCount_ = 0;
    uint16_t warnEvents_ = 0;
    uint16_t errorEvents_ = 0;

    double sumScore_ = 0, sumQuality_ = 0, sumDev_ = 0, sumAp_ = 0, sumBle_ = 0, sumPress_ = 0, sumRssi_ = 0;
    uint8_t minScore_ = 100, maxScore_ = 0;
    uint8_t minQuality_ = 100, maxQuality_ = 0;

    Sample ring_[kMaxSamples];
    size_t head_ = 0;
    size_t ringCount_ = 0;

    Phase phases_[kMaxPhases];
    uint8_t phaseCount_ = 0;
};
