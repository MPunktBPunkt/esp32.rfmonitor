#pragma once

#include <stdint.h>

// Pure score models — no WiFi/Arduino deps so they can be unit-tested later.
struct RfScoreFeatures {
    int8_t ownRssi = 0;
    float rssiJitter = 0;
    uint8_t densityProxy = 0;      // 0..80-ish from AP count
    uint8_t pressureProxy = 0;     // max APs/channel heuristic
    uint8_t occupancyProxy = 0;    // occupied channels heuristic
    uint8_t stability = 100;       // 0..100
    bool connected = true;
    uint8_t measurementQuality = 100;
};

struct RfScoreBreakdown {
    uint8_t ownLink = 0;
    uint8_t jitter = 0;
    uint8_t density = 0;
    uint8_t pressure = 0;
    uint8_t occupancy = 0;
    uint8_t stability = 0;
    uint8_t disconnect = 0;
    uint8_t qualityGate = 0;
};

struct RfScoreResult {
    uint8_t score = 0;
    uint8_t version = 0;
    RfScoreBreakdown parts;
};

namespace RfScore {
RfScoreResult computeV1(const RfScoreFeatures& f);
RfScoreResult computeV2(const RfScoreFeatures& f);
}
