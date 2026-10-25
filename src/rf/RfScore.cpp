#include "RfScore.h"

static uint8_t clampU8(int v) {
    if (v < 0) return 0;
    if (v > 100) return 100;
    return (uint8_t)v;
}

RfScoreResult RfScore::computeV1(const RfScoreFeatures& f) {
    RfScoreResult r;
    r.version = 1;
    r.parts.ownLink = f.ownRssi < -55 ? (uint8_t)((-55 - f.ownRssi) * 1.2f) : 0;
    r.parts.jitter = f.rssiJitter > 8 ? (uint8_t)((f.rssiJitter - 8) * 2) : 0;
    r.parts.pressure = f.pressureProxy / 4;
    r.parts.density = f.densityProxy / 6;
    r.parts.occupancy = 0;
    r.parts.stability = (100 - f.stability) / 3;
    r.parts.disconnect = f.connected ? 0 : 25;
    r.parts.qualityGate = 0;
    int score = 100 - (int)r.parts.ownLink - (int)r.parts.jitter - (int)r.parts.pressure
                - (int)r.parts.density - (int)r.parts.stability - (int)r.parts.disconnect;
    r.score = clampU8(score);
    return r;
}

RfScoreResult RfScore::computeV2(const RfScoreFeatures& f) {
    // v2: softer environment proxies, uses occupancy lightly, quality gate when
    // observation is weak. Still heuristic — not airtime / interference.
    RfScoreResult r;
    r.version = 2;

    // Own link: threshold -50 (was -55), gentler slope.
    r.parts.ownLink = f.ownRssi < -50 ? (uint8_t)((-50 - f.ownRssi) * 1.0f) : 0;
    // Jitter: start later, milder.
    r.parts.jitter = f.rssiJitter > 10 ? (uint8_t)((f.rssiJitter - 10) * 1.5f) : 0;
    // Proxies: less aggressive than v1 so dense sites don't always look "broken".
    r.parts.density = f.densityProxy / 8;
    r.parts.pressure = f.pressureProxy / 5;
    r.parts.occupancy = f.occupancyProxy / 20;
    r.parts.stability = (100 - f.stability) / 4;
    r.parts.disconnect = f.connected ? 0 : 20;
    // If we barely trust the observation, don't pretend the score is solid.
    r.parts.qualityGate = f.measurementQuality < 40 ? 10 : (f.measurementQuality < 60 ? 5 : 0);

    int score = 100 - (int)r.parts.ownLink - (int)r.parts.jitter
                - (int)r.parts.density - (int)r.parts.pressure - (int)r.parts.occupancy
                - (int)r.parts.stability - (int)r.parts.disconnect - (int)r.parts.qualityGate;
    r.score = clampU8(score);
    return r;
}
