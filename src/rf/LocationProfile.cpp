#include "LocationProfile.h"
#include <Preferences.h>
#include <math.h>
#include <string.h>

static Preferences prefs;

void LocationProfile::begin(ConfigStore* cfg) {
    cfg_ = cfg;
    load();
}

void LocationProfile::load() {
    prefs.begin("rfmon_loc", true);
    valid_ = prefs.getBool("valid", false);
    if (!valid_) {
        prefs.end();
        return;
    }
    capturedAtS_ = prefs.getUInt("at", 0);
    sampleCount_ = prefs.getUShort("n", 0);
    baseScore_ = prefs.getFloat("score", 0);
    baseDensity_ = prefs.getFloat("dens", 0);
    basePressure_ = prefs.getFloat("press", 0);
    baseOccupancy_ = prefs.getFloat("occ", 0);
    baseApCount_ = prefs.getFloat("aps", 0);
    baseBleCount_ = prefs.getFloat("ble", 0);
    baseOwnRssi_ = prefs.getFloat("rssi", 0);
    for (uint8_t i = 0; i < 14; i++) {
        char key[6];
        snprintf(key, sizeof(key), "c%u", (unsigned)i);
        baseChannels_[i] = prefs.getFloat(key, 0);
    }
    prefs.end();
}

void LocationProfile::persist() {
    prefs.begin("rfmon_loc", false);
    prefs.putBool("valid", valid_);
    prefs.putUInt("at", capturedAtS_);
    prefs.putUShort("n", sampleCount_);
    prefs.putFloat("score", baseScore_);
    prefs.putFloat("dens", baseDensity_);
    prefs.putFloat("press", basePressure_);
    prefs.putFloat("occ", baseOccupancy_);
    prefs.putFloat("aps", baseApCount_);
    prefs.putFloat("ble", baseBleCount_);
    prefs.putFloat("rssi", baseOwnRssi_);
    for (uint8_t i = 0; i < 14; i++) {
        char key[6];
        snprintf(key, sizeof(key), "c%u", (unsigned)i);
        prefs.putFloat(key, baseChannels_[i]);
    }
    prefs.end();
}

bool LocationProfile::clear() {
    capturing_ = false;
    valid_ = false;
    sampleCount_ = 0;
    prefs.begin("rfmon_loc", false);
    prefs.clear();
    prefs.end();
    deviation_ = 0;
    return true;
}

bool LocationProfile::startCapture(uint16_t durationS) {
    if (durationS < 60) durationS = 60;
    if (durationS > 3600) durationS = 3600;
    captureTargetS_ = durationS;
    capturing_ = true;
    captureStartMs_ = millis();
    lastSampleMs_ = 0;
    captureCount_ = 0;
    sumScore_ = sumDensity_ = sumPressure_ = sumOccupancy_ = 0;
    sumAp_ = sumBle_ = sumRssi_ = 0;
    memset(sumCh_, 0, sizeof(sumCh_));
    return true;
}

void LocationProfile::cancelCapture() {
    capturing_ = false;
    captureCount_ = 0;
}

uint16_t LocationProfile::captureProgressPct() const {
    if (!capturing_ || captureTargetS_ == 0) return 0;
    unsigned long elapsed = (millis() - captureStartMs_) / 1000UL;
    if (elapsed >= captureTargetS_) return 100;
    return (uint16_t)((elapsed * 100UL) / captureTargetS_);
}

void LocationProfile::addCaptureSample(uint8_t score, uint8_t density, uint8_t pressure, uint8_t occupancy,
                                       uint8_t apCount, uint8_t bleCount, float ownRssi,
                                       const uint8_t channels[14]) {
    sumScore_ += score;
    sumDensity_ += density;
    sumPressure_ += pressure;
    sumOccupancy_ += occupancy;
    sumAp_ += apCount;
    sumBle_ += bleCount;
    sumRssi_ += ownRssi;
    for (uint8_t i = 0; i < 14; i++) sumCh_[i] += channels[i];
    captureCount_++;
}

void LocationProfile::finalizeCapture() {
    if (captureCount_ == 0) {
        capturing_ = false;
        return;
    }
    float n = (float)captureCount_;
    baseScore_ = (float)(sumScore_ / n);
    baseDensity_ = (float)(sumDensity_ / n);
    basePressure_ = (float)(sumPressure_ / n);
    baseOccupancy_ = (float)(sumOccupancy_ / n);
    baseApCount_ = (float)(sumAp_ / n);
    baseBleCount_ = (float)(sumBle_ / n);
    baseOwnRssi_ = (float)(sumRssi_ / n);
    for (uint8_t i = 0; i < 14; i++) baseChannels_[i] = (float)(sumCh_[i] / n);
    sampleCount_ = captureCount_;
    capturedAtS_ = millis() / 1000UL;
    valid_ = true;
    capturing_ = false;
    persist();
}

bool LocationProfile::saveNowFromAverages() {
    if (!capturing_ || captureCount_ < 3) return false;
    finalizeCapture();
    return valid_;
}

void LocationProfile::recomputeDeviation(uint8_t score, uint8_t density, uint8_t pressure,
                                         uint8_t apCount, uint8_t bleCount, float ownRssi) {
    if (!valid_) {
        deviation_ = 0;
        return;
    }
    // Weighted blend: score dominates, then density/pressure/ap proxies, then RSSI.
    float dScore = (float)score - baseScore_;
    float dDens = baseDensity_ - (float)density;       // more density than baseline → negative
    float dPress = basePressure_ - (float)pressure;
    float dAp = baseApCount_ - (float)apCount;
    float dBle = baseBleCount_ - (float)bleCount;
    float dRssi = ownRssi - baseOwnRssi_;               // weaker RSSI → negative

    float raw = 0.45f * dScore
              + 0.15f * (dDens / 2.0f)
              + 0.15f * (dPress / 2.0f)
              + 0.10f * (dAp * 2.0f)
              + 0.05f * (dBle * 2.0f)
              + 0.10f * dRssi;
    if (raw > 100) raw = 100;
    if (raw < -100) raw = -100;
    deviation_ = (int8_t)lroundf(raw);
}

void LocationProfile::loop(uint8_t score, uint8_t density, uint8_t pressure, uint8_t occupancy,
                           uint8_t apCount, uint8_t bleCount, float ownRssi, const uint8_t channels[14]) {
    if (capturing_) {
        if (millis() - lastSampleMs_ >= 4000UL) {
            lastSampleMs_ = millis();
            addCaptureSample(score, density, pressure, occupancy, apCount, bleCount, ownRssi, channels);
        }
        if ((millis() - captureStartMs_) / 1000UL >= captureTargetS_) {
            finalizeCapture();
        }
    }
    recomputeDeviation(score, density, pressure, apCount, bleCount, ownRssi);
}

void LocationProfile::appendJson(JsonObject target) const {
    target["valid"] = valid_;
    target["capturing"] = capturing_;
    target["captureProgressPct"] = captureProgressPct();
    target["captureSamples"] = captureCount_;
    target["captureTargetS"] = captureTargetS_;
    target["deviation"] = deviation_;
    if (!valid_) return;
    target["capturedAtS"] = capturedAtS_;
    target["sampleCount"] = sampleCount_;
    JsonObject b = target["baseline"].to<JsonObject>();
    b["score"] = baseScore_;
    b["density"] = baseDensity_;
    b["channelPressure"] = basePressure_;
    b["channelOccupancy"] = baseOccupancy_;
    b["apCount"] = baseApCount_;
    b["bleCount"] = baseBleCount_;
    b["ownRssi"] = baseOwnRssi_;
    JsonArray ch = b["channels"].to<JsonArray>();
    for (uint8_t i = 0; i < 14; i++) ch.add(baseChannels_[i]);
}
