#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "core/ConfigStore.h"

// Standortnormal (NVS). Deviation = aktuelle Lage vs. dieses Profil — nicht Score-Offset.
class LocationProfile {
public:
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
    void appendJson(JsonObject target) const;

private:
    void load();
    void persist();
    void recomputeDeviation(uint8_t score, uint8_t density, uint8_t pressure,
                            uint8_t apCount, uint8_t bleCount, float ownRssi);
    void addCaptureSample(uint8_t score, uint8_t density, uint8_t pressure, uint8_t occupancy,
                          uint8_t apCount, uint8_t bleCount, float ownRssi, const uint8_t channels[14]);
    void finalizeCapture();

    ConfigStore* cfg_ = nullptr;
    bool valid_ = false;
    bool capturing_ = false;
    uint16_t captureTargetS_ = 300;
    unsigned long captureStartMs_ = 0;
    unsigned long lastSampleMs_ = 0;
    uint16_t captureCount_ = 0;

    // Stored baseline
    uint32_t capturedAtS_ = 0;
    uint16_t sampleCount_ = 0;
    float baseScore_ = 0;
    float baseDensity_ = 0;
    float basePressure_ = 0;
    float baseOccupancy_ = 0;
    float baseApCount_ = 0;
    float baseBleCount_ = 0;
    float baseOwnRssi_ = 0;
    float baseChannels_[14] = {0};

    // Capture accumulators
    double sumScore_ = 0, sumDensity_ = 0, sumPressure_ = 0, sumOccupancy_ = 0;
    double sumAp_ = 0, sumBle_ = 0, sumRssi_ = 0;
    double sumCh_[14] = {0};

    int8_t deviation_ = 0;
};
