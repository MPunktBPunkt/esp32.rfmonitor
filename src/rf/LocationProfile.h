#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "core/ConfigStore.h"

// Standortnormal (NVS). Deviation = aktuelle Lage vs. dieses Profil — nicht Score-Offset.
// profileVersion 2: mean + stddev + min/max. deviationVersion 1: gewichtete Heuristik + Teile.
class LocationProfile {
public:
    static constexpr uint8_t kProfileVersion = 2;
    static constexpr uint8_t kDeviationVersion = 1;
    static constexpr uint32_t kStaleAfterS = 90UL * 24UL * 3600UL;  // 90 days

    void begin(ConfigStore* cfg);
    void loop(uint8_t score, uint8_t density, uint8_t pressure, uint8_t occupancy,
              uint8_t apCount, uint8_t bleCount, float ownRssi, const uint8_t channels[14]);

    bool valid() const { return valid_; }
    bool capturing() const { return capturing_; }
    uint16_t captureProgressPct() const;
    uint16_t captureSamples() const { return captureCount_; }
    uint16_t captureTargetS() const { return captureTargetS_; }

    bool startCapture(uint16_t durationS = 300);
    void cancelCapture();
    bool clear();
    bool saveNowFromAverages();  // finalize current capture window early

    // Deviation: -100..+100, negativ = schlechter als Standortnormal.
    int8_t deviation() const { return deviation_; }
    bool stale() const;
    void appendJson(JsonObject target) const;

private:
    struct Stat {
        float mean = 0;
        float stddev = 0;
        float minV = 0;
        float maxV = 0;
    };
    struct DevParts {
        float score = 0;
        float density = 0;
        float pressure = 0;
        float apCount = 0;
        float bleCount = 0;
        float rssi = 0;
    };

    void load();
    void persist();
    void recomputeDeviation(uint8_t score, uint8_t density, uint8_t pressure,
                            uint8_t apCount, uint8_t bleCount, float ownRssi);
    void addCaptureSample(uint8_t score, uint8_t density, uint8_t pressure, uint8_t occupancy,
                          uint8_t apCount, uint8_t bleCount, float ownRssi, const uint8_t channels[14]);
    void finalizeCapture();
    static void putStat(JsonObject parent, const char* key, const Stat& s);
    static float stdFromSums(double sum, double sumSq, float n);

    ConfigStore* cfg_ = nullptr;
    bool valid_ = false;
    bool capturing_ = false;
    uint16_t captureTargetS_ = 300;
    unsigned long captureStartMs_ = 0;
    unsigned long lastSampleMs_ = 0;
    uint16_t captureCount_ = 0;

    // Stored baseline
    uint32_t capturedAtUptimeS_ = 0;
    uint32_t capturedAtEpochS_ = 0;
    bool capturedTimeValid_ = false;
    uint16_t sampleCount_ = 0;
    Stat score_{}, density_{}, pressure_{}, occupancy_{};
    Stat apCount_{}, bleCount_{}, ownRssi_{};
    float baseChannels_[14] = {0};

    // Capture accumulators (Welford via sum + sumSq)
    double sumScore_ = 0, sumSqScore_ = 0;
    double sumDensity_ = 0, sumSqDensity_ = 0;
    double sumPressure_ = 0, sumSqPressure_ = 0;
    double sumOccupancy_ = 0, sumSqOccupancy_ = 0;
    double sumAp_ = 0, sumSqAp_ = 0;
    double sumBle_ = 0, sumSqBle_ = 0;
    double sumRssi_ = 0, sumSqRssi_ = 0;
    double sumCh_[14] = {0};
    float minScore_ = 0, maxScore_ = 0;
    float minDens_ = 0, maxDens_ = 0;
    float minPress_ = 0, maxPress_ = 0;
    float minOcc_ = 0, maxOcc_ = 0;
    float minAp_ = 0, maxAp_ = 0;
    float minBle_ = 0, maxBle_ = 0;
    float minRssi_ = 0, maxRssi_ = 0;

    int8_t deviation_ = 0;
    DevParts parts_{};
};
