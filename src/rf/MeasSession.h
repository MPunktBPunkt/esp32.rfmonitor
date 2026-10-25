#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "BuildFlags.h"

// Reproducible field measurement window (summary + ring of samples).
class MeasSession {
public:
    void begin();
    bool active() const { return active_; }
    bool start(const char* label = "");
    bool stop();
    void noteSample(uint8_t score, uint8_t quality, int8_t deviation,
                    uint8_t apCount, uint8_t bleCount, uint8_t pressure,
                    uint8_t occupancy, uint8_t density, float jitter, uint8_t stability,
                    bool connected, float ownRssi, float radioDuty, uint8_t eventLevel);
    void appendStatusJson(JsonObject target) const;
    void appendExportJson(JsonObject target) const;

private:
    static constexpr size_t kMaxSamples = 90;  // ~6 min @ 4s on Mini; S3 same (RAM-safe)

    struct Sample {
        uint32_t tsUptime;
        uint32_t tsEpoch;  // 0 if time invalid at sample
        uint8_t score;
        uint8_t quality;
        int8_t deviation;
        uint8_t apCount;
        uint8_t bleCount;
        uint8_t pressure;
        uint8_t occupancy;
        uint8_t density;
        uint8_t jitter;     // dB, clamped
        uint8_t stability;
        uint8_t flags;      // bit0 = connected
        int8_t ownRssi;
        uint8_t radioDuty;
    };

    bool active_ = false;
    char label_[32] = {0};
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
};
