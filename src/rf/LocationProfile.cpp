#include "LocationProfile.h"
#include "core/TimeBase.h"
#include <Preferences.h>
#include <math.h>
#include <string.h>

static Preferences prefs;

void LocationProfile::begin(ConfigStore* cfg) {
    cfg_ = cfg;
    load();
}

float LocationProfile::stdFromSums(double sum, double sumSq, float n) {
    if (n < 2.0f) return 0;
    double mean = sum / n;
    double var = (sumSq / n) - (mean * mean);
    if (var < 0) var = 0;
    return (float)sqrt(var);
}

void LocationProfile::load() {
    prefs.begin("rfmon_loc", true);
    valid_ = prefs.getBool("valid", false);
    if (!valid_) {
        prefs.end();
        return;
    }
    capturedAtUptimeS_ = prefs.getUInt("at", 0);
    capturedAtEpochS_ = prefs.getUInt("atEp", 0);
    capturedTimeValid_ = prefs.getBool("tVal", false);
    sampleCount_ = prefs.getUShort("n", 0);

    auto loadStat = [&](Stat& s, const char* m, const char* sd, const char* mn, const char* mx) {
        s.mean = prefs.getFloat(m, 0);
        s.stddev = prefs.getFloat(sd, 0);
        s.minV = prefs.getFloat(mn, s.mean);
        s.maxV = prefs.getFloat(mx, s.mean);
    };
    loadStat(score_, "score", "sSd", "sMn", "sMx");
    loadStat(density_, "dens", "dSd", "dMn", "dMx");
    loadStat(pressure_, "press", "pSd", "pMn", "pMx");
    loadStat(occupancy_, "occ", "oSd", "oMn", "oMx");
    loadStat(apCount_, "aps", "aSd", "aMn", "aMx");
    loadStat(bleCount_, "ble", "bSd", "bMn", "bMx");
    loadStat(ownRssi_, "rssi", "rSd", "rMn", "rMx");
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
    prefs.putUChar("pVer", kProfileVersion);
    prefs.putUInt("at", capturedAtUptimeS_);
    prefs.putUInt("atEp", capturedAtEpochS_);
    prefs.putBool("tVal", capturedTimeValid_);
    prefs.putUShort("n", sampleCount_);

    auto putStat = [&](const Stat& s, const char* m, const char* sd, const char* mn, const char* mx) {
        prefs.putFloat(m, s.mean);
        prefs.putFloat(sd, s.stddev);
        prefs.putFloat(mn, s.minV);
        prefs.putFloat(mx, s.maxV);
    };
    putStat(score_, "score", "sSd", "sMn", "sMx");
    putStat(density_, "dens", "dSd", "dMn", "dMx");
    putStat(pressure_, "press", "pSd", "pMn", "pMx");
    putStat(occupancy_, "occ", "oSd", "oMn", "oMx");
    putStat(apCount_, "aps", "aSd", "aMn", "aMx");
    putStat(bleCount_, "ble", "bSd", "bMn", "bMx");
    putStat(ownRssi_, "rssi", "rSd", "rMn", "rMx");
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
    capturedAtEpochS_ = 0;
    capturedTimeValid_ = false;
    prefs.begin("rfmon_loc", false);
    prefs.clear();
    prefs.end();
    deviation_ = 0;
    parts_ = {};
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
    sumScore_ = sumSqScore_ = 0;
    sumDensity_ = sumSqDensity_ = 0;
    sumPressure_ = sumSqPressure_ = 0;
    sumOccupancy_ = sumSqOccupancy_ = 0;
    sumAp_ = sumSqAp_ = 0;
    sumBle_ = sumSqBle_ = 0;
    sumRssi_ = sumSqRssi_ = 0;
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
    auto accum = [](double& sum, double& sumSq, float& mn, float& mx, uint16_t n, float v) {
        sum += v;
        sumSq += (double)v * (double)v;
        if (n == 0) {
            mn = mx = v;
        } else {
            if (v < mn) mn = v;
            if (v > mx) mx = v;
        }
    };
    accum(sumScore_, sumSqScore_, minScore_, maxScore_, captureCount_, score);
    accum(sumDensity_, sumSqDensity_, minDens_, maxDens_, captureCount_, density);
    accum(sumPressure_, sumSqPressure_, minPress_, maxPress_, captureCount_, pressure);
    accum(sumOccupancy_, sumSqOccupancy_, minOcc_, maxOcc_, captureCount_, occupancy);
    accum(sumAp_, sumSqAp_, minAp_, maxAp_, captureCount_, apCount);
    accum(sumBle_, sumSqBle_, minBle_, maxBle_, captureCount_, bleCount);
    accum(sumRssi_, sumSqRssi_, minRssi_, maxRssi_, captureCount_, ownRssi);
    for (uint8_t i = 0; i < 14; i++) sumCh_[i] += channels[i];
    captureCount_++;
}

void LocationProfile::finalizeCapture() {
    if (captureCount_ == 0) {
        capturing_ = false;
        return;
    }
    float n = (float)captureCount_;
    auto fin = [&](Stat& s, double sum, double sumSq, float mn, float mx) {
        s.mean = (float)(sum / n);
        s.stddev = stdFromSums(sum, sumSq, n);
        s.minV = mn;
        s.maxV = mx;
    };
    fin(score_, sumScore_, sumSqScore_, minScore_, maxScore_);
    fin(density_, sumDensity_, sumSqDensity_, minDens_, maxDens_);
    fin(pressure_, sumPressure_, sumSqPressure_, minPress_, maxPress_);
    fin(occupancy_, sumOccupancy_, sumSqOccupancy_, minOcc_, maxOcc_);
    fin(apCount_, sumAp_, sumSqAp_, minAp_, maxAp_);
    fin(bleCount_, sumBle_, sumSqBle_, minBle_, maxBle_);
    fin(ownRssi_, sumRssi_, sumSqRssi_, minRssi_, maxRssi_);
    for (uint8_t i = 0; i < 14; i++) baseChannels_[i] = (float)(sumCh_[i] / n);
    sampleCount_ = captureCount_;
    capturedAtUptimeS_ = TimeBase::uptimeS();
    capturedTimeValid_ = TimeBase::timeValid();
    capturedAtEpochS_ = capturedTimeValid_ ? TimeBase::epochS() : 0;
    valid_ = true;
    capturing_ = false;
    persist();
}

bool LocationProfile::saveNowFromAverages() {
    if (!capturing_ || captureCount_ < 3) return false;
    finalizeCapture();
    return valid_;
}

bool LocationProfile::stale() const {
    if (!valid_ || !capturedTimeValid_ || capturedAtEpochS_ == 0) return false;
    if (!TimeBase::timeValid()) return false;
    uint32_t now = TimeBase::epochS();
    if (now < capturedAtEpochS_) return false;
    return (now - capturedAtEpochS_) >= kStaleAfterS;
}

void LocationProfile::recomputeDeviation(uint8_t score, uint8_t density, uint8_t pressure,
                                         uint8_t apCount, uint8_t bleCount, float ownRssi) {
    if (!valid_) {
        deviation_ = 0;
        driftMag_ = 0;
        zScore_ = zAp_ = zRssi_ = 0;
        parts_ = {};
        return;
    }
    // Weighted blend (deviationVersion 1). More density/pressure/AP than baseline → negative.
    parts_.score = 0.45f * ((float)score - score_.mean);
    parts_.density = 0.15f * ((density_.mean - (float)density) / 2.0f);
    parts_.pressure = 0.15f * ((pressure_.mean - (float)pressure) / 2.0f);
    parts_.apCount = 0.10f * ((apCount_.mean - (float)apCount) * 2.0f);
    parts_.bleCount = 0.05f * ((bleCount_.mean - (float)bleCount) * 2.0f);
    parts_.rssi = 0.10f * (ownRssi - ownRssi_.mean);

    float raw = parts_.score + parts_.density + parts_.pressure
              + parts_.apCount + parts_.bleCount + parts_.rssi;
    if (raw > 100) raw = 100;
    if (raw < -100) raw = -100;
    deviation_ = (int8_t)lroundf(raw);

    // Drift = how far current sits from the captured distribution (unsigned).
    driftMag_ = fabsf(raw);
    auto z = [](float cur, const Stat& s) -> float {
        float denom = s.stddev > 0.5f ? s.stddev : 0.5f;
        return (cur - s.mean) / denom;
    };
    zScore_ = z((float)score, score_);
    zAp_ = z((float)apCount, apCount_);
    zRssi_ = z(ownRssi, ownRssi_);
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

void LocationProfile::putStat(JsonObject parent, const char* key, const Stat& s) {
    JsonObject o = parent[key].to<JsonObject>();
    o["mean"] = s.mean;
    o["stddev"] = s.stddev;
    o["min"] = s.minV;
    o["max"] = s.maxV;
}

void LocationProfile::appendJson(JsonObject target) const {
    target["valid"] = valid_;
    target["capturing"] = capturing_;
    target["captureProgressPct"] = captureProgressPct();
    target["captureSamples"] = captureCount_;
    target["captureTargetS"] = captureTargetS_;
    target["deviation"] = deviation_;
    target["deviationVersion"] = kDeviationVersion;
    target["profileVersion"] = kProfileVersion;
    target["stale"] = stale();
    if (!valid_) return;

    target["capturedAtS"] = capturedAtUptimeS_;  // legacy alias = uptime at capture
    target["capturedAtUptimeS"] = capturedAtUptimeS_;
    target["capturedAtEpochS"] = capturedAtEpochS_;
    target["capturedTimeValid"] = capturedTimeValid_;
    target["sampleCount"] = sampleCount_;
    if (capturedTimeValid_ && capturedAtEpochS_ > 0 && TimeBase::timeValid()) {
        uint32_t now = TimeBase::epochS();
        target["baselineAgeS"] = now >= capturedAtEpochS_ ? (now - capturedAtEpochS_) : 0;
    }

    JsonObject b = target["baseline"].to<JsonObject>();
    // Flat means kept for older UI / evidence tools.
    b["score"] = score_.mean;
    b["density"] = density_.mean;
    b["channelPressure"] = pressure_.mean;
    b["channelOccupancy"] = occupancy_.mean;
    b["apCount"] = apCount_.mean;
    b["bleCount"] = bleCount_.mean;
    b["ownRssi"] = ownRssi_.mean;
    JsonArray ch = b["channels"].to<JsonArray>();
    for (uint8_t i = 0; i < 14; i++) ch.add(baseChannels_[i]);

    JsonObject dist = target["distribution"].to<JsonObject>();
    putStat(dist, "score", score_);
    putStat(dist, "density", density_);
    putStat(dist, "channelPressure", pressure_);
    putStat(dist, "channelOccupancy", occupancy_);
    putStat(dist, "apCount", apCount_);
    putStat(dist, "bleCount", bleCount_);
    putStat(dist, "ownRssi", ownRssi_);

    JsonObject dp = target["deviationParts"].to<JsonObject>();
    dp["score"] = parts_.score;
    dp["density"] = parts_.density;
    dp["pressure"] = parts_.pressure;
    dp["apCount"] = parts_.apCount;
    dp["bleCount"] = parts_.bleCount;
    dp["rssi"] = parts_.rssi;

    JsonObject drift = target["drift"].to<JsonObject>();
    drift["magnitude"] = driftMag_;
    drift["stale"] = stale();
    drift["zScore"] = zScore_;
    drift["zApCount"] = zAp_;
    drift["zOwnRssi"] = zRssi_;
    // Soft flag: outside ~2σ on score or AP, or absolute Δ large.
    drift["elevated"] = (fabsf(zScore_) >= 2.0f) || (fabsf(zAp_) >= 2.0f) || (driftMag_ >= 20.0f);
}
