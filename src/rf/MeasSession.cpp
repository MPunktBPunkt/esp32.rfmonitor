#include "MeasSession.h"
#include "core/TimeBase.h"
#include <string.h>
#include <math.h>

void MeasSession::begin() {
    active_ = false;
    sampleCount_ = 0;
    ringCount_ = 0;
    head_ = 0;
    phaseCount_ = 0;
    currentPhaseId_[0] = 0;
}

void MeasSession::closeOpenPhase(uint32_t sampleIdx, uint32_t uptimeS) {
    if (phaseCount_ == 0) return;
    Phase& p = phases_[phaseCount_ - 1];
    if (!p.open) return;
    p.open = false;
    p.endSample = sampleIdx;
    p.endUptimeS = uptimeS;
}

void MeasSession::accumulatePhase(Phase& p, uint8_t score, uint8_t quality, int8_t deviation,
                                  uint8_t apCount, uint8_t bleCount, uint8_t pressure,
                                  uint8_t occupancy, uint8_t density, float jitter, uint8_t stability,
                                  bool connected, float ownRssi, float radioDuty) {
    p.n++;
    if (connected) p.connectedN++;
    p.sumScore += score;
    p.sumQuality += quality;
    p.sumDev += deviation;
    p.sumAp += apCount;
    p.sumBle += bleCount;
    p.sumPress += pressure;
    p.sumOcc += occupancy;
    p.sumDens += density;
    p.sumJitter += jitter;
    p.sumStab += stability;
    p.sumRssi += ownRssi;
    p.sumDuty += radioDuty;
    if (score < p.minScore) p.minScore = score;
    if (score > p.maxScore) p.maxScore = score;
}

void MeasSession::appendPhaseJson(JsonObject o, const Phase& p) const {
    o["id"] = p.id;
    o["note"] = p.note;
    o["startSample"] = p.startSample;
    o["endSample"] = p.open ? sampleCount_ : p.endSample;
    o["startUptimeS"] = p.startUptimeS;
    o["endUptimeS"] = p.open ? TimeBase::uptimeS() : p.endUptimeS;
    o["open"] = p.open;
    o["n"] = p.n;
    if (p.n == 0) return;
    float n = (float)p.n;
    o["avgScore"] = p.sumScore / n;
    o["avgQuality"] = p.sumQuality / n;
    o["avgDeviation"] = p.sumDev / n;
    o["avgApCount"] = p.sumAp / n;
    o["avgBleCount"] = p.sumBle / n;
    o["avgPressure"] = p.sumPress / n;
    o["avgOccupancy"] = p.sumOcc / n;
    o["avgDensity"] = p.sumDens / n;
    o["avgJitter"] = p.sumJitter / n;
    o["avgStability"] = p.sumStab / n;
    o["avgOwnRssi"] = p.sumRssi / n;
    o["avgRadioDuty"] = p.sumDuty / n;
    o["connectedPct"] = (p.connectedN * 100.0f) / n;
    o["minScore"] = p.minScore;
    o["maxScore"] = p.maxScore;
}

bool MeasSession::start(const char* label, const SessionProfileDef* profile, uint16_t durationOverrideS) {
    active_ = true;
    startS_ = TimeBase::uptimeS();
    endS_ = 0;
    startTimeValid_ = TimeBase::timeValid();
    startEpochS_ = startTimeValid_ ? TimeBase::epochS() : 0;
    endEpochS_ = 0;
    endTimeValid_ = false;
    sampleCount_ = 0;
    warnEvents_ = 0;
    errorEvents_ = 0;
    sumScore_ = sumQuality_ = sumDev_ = sumAp_ = sumBle_ = sumPress_ = sumRssi_ = 0;
    minScore_ = 100;
    maxScore_ = 0;
    minQuality_ = 100;
    maxQuality_ = 0;
    head_ = 0;
    ringCount_ = 0;
    phaseCount_ = 0;
    currentPhaseId_[0] = 0;

    memset(label_, 0, sizeof(label_));
    memset(profileId_, 0, sizeof(profileId_));
    memset(profileTitle_, 0, sizeof(profileTitle_));

    const SessionProfileDef& p = profile ? *profile : SessionProfile::field();
    strncpy(profileId_, p.id, sizeof(profileId_) - 1);
    strncpy(profileTitle_, p.title, sizeof(profileTitle_) - 1);
    durationS_ = durationOverrideS ? durationOverrideS : p.durationS;
    wifiIntervalS_ = p.wifiScanIntervalS;
    bleIntervalS_ = p.bleScanIntervalS;
    bleEnabled_ = p.bleEnabled;
    characterization_ = p.characterization;

    if (label && label[0]) {
        strncpy(label_, label, sizeof(label_) - 1);
    } else {
        snprintf(label_, sizeof(label_), "%s-%lu", p.id, (unsigned long)startS_);
    }
    return true;
}

bool MeasSession::stop() {
    if (!active_) return false;
    active_ = false;
    endS_ = TimeBase::uptimeS();
    endTimeValid_ = TimeBase::timeValid();
    endEpochS_ = endTimeValid_ ? TimeBase::epochS() : 0;
    closeOpenPhase(sampleCount_, endS_);
    currentPhaseId_[0] = 0;
    return true;
}

bool MeasSession::markPhase(const char* id, const char* note) {
    if (!active_ || !id || !id[0]) return false;
    uint32_t now = TimeBase::uptimeS();
    closeOpenPhase(sampleCount_, now);
    if (phaseCount_ >= kMaxPhases) return false;
    Phase& p = phases_[phaseCount_++];
    memset(&p, 0, sizeof(p));
    strncpy(p.id, id, sizeof(p.id) - 1);
    if (note && note[0]) strncpy(p.note, note, sizeof(p.note) - 1);
    p.startSample = sampleCount_;
    p.startUptimeS = now;
    p.open = true;
    p.minScore = 100;
    p.maxScore = 0;
    strncpy(currentPhaseId_, p.id, sizeof(currentPhaseId_) - 1);
    return true;
}

bool MeasSession::loopAutoStop() {
    if (!active_ || durationS_ == 0) return false;
    if (TimeBase::uptimeS() - startS_ < durationS_) return false;
    stop();
    return true;
}

void MeasSession::noteSample(uint8_t score, uint8_t quality, int8_t deviation,
                             uint8_t apCount, uint8_t bleCount, uint8_t pressure,
                             uint8_t occupancy, uint8_t density, float jitter, uint8_t stability,
                             bool connected, float ownRssi, float radioDuty, uint8_t eventLevel) {
    if (!active_) return;
    sampleCount_++;
    sumScore_ += score;
    sumQuality_ += quality;
    sumDev_ += deviation;
    sumAp_ += apCount;
    sumBle_ += bleCount;
    sumPress_ += pressure;
    sumRssi_ += ownRssi;
    if (score < minScore_) minScore_ = score;
    if (score > maxScore_) maxScore_ = score;
    if (quality < minQuality_) minQuality_ = quality;
    if (quality > maxQuality_) maxQuality_ = quality;
    if (eventLevel == 1) warnEvents_++;
    if (eventLevel == 0) errorEvents_++;

    if (phaseCount_ > 0) {
        Phase& ph = phases_[phaseCount_ - 1];
        if (ph.open) {
            accumulatePhase(ph, score, quality, deviation, apCount, bleCount, pressure,
                            occupancy, density, jitter, stability, connected, ownRssi, radioDuty);
        }
    }

    Sample& s = ring_[head_];
    s.tsUptime = TimeBase::uptimeS();
    s.tsEpoch = TimeBase::timeValid() ? TimeBase::epochS() : 0;
    s.score = score;
    s.quality = quality;
    s.deviation = deviation;
    s.apCount = apCount;
    s.bleCount = bleCount;
    s.pressure = pressure;
    s.occupancy = occupancy;
    s.density = density;
    s.jitter = jitter > 255 ? 255 : (uint8_t)lroundf(jitter);
    s.stability = stability;
    s.flags = connected ? 0x01 : 0x00;
    s.ownRssi = (int8_t)lroundf(ownRssi);
    s.radioDuty = radioDuty > 255 ? 255 : (uint8_t)lroundf(radioDuty);
    memset(s.phase, 0, sizeof(s.phase));
    if (currentPhaseId_[0]) strncpy(s.phase, currentPhaseId_, sizeof(s.phase) - 1);
    head_ = (head_ + 1) % kMaxSamples;
    if (ringCount_ < kMaxSamples) ringCount_++;
}

void MeasSession::appendStatusJson(JsonObject target) const {
    target["active"] = active_;
    target["label"] = label_;
    target["profile"] = profileId_;
    target["profileTitle"] = profileTitle_;
    target["durationS"] = durationS_;
    target["wifiScanIntervalS"] = wifiIntervalS_;
    target["bleScanIntervalS"] = bleIntervalS_;
    target["bleEnabled"] = bleEnabled_;
    target["characterization"] = characterization_;
    target["phase"] = currentPhaseId_;
    target["startS"] = startS_;
    target["endS"] = endS_;
    target["startEpochS"] = startEpochS_;
    target["endEpochS"] = endEpochS_;
    target["timeValid"] = startTimeValid_;
    target["sampleCount"] = sampleCount_;
    target["ringCount"] = (uint32_t)ringCount_;
    target["ringCapacity"] = (uint32_t)kMaxSamples;
    target["warnEvents"] = warnEvents_;
    target["errorEvents"] = errorEvents_;
    if (active_ && durationS_ > 0) {
        uint32_t elapsed = TimeBase::uptimeS() - startS_;
        target["elapsedS"] = elapsed;
        target["remainingS"] = elapsed >= durationS_ ? 0 : (durationS_ - elapsed);
        target["progressPct"] = elapsed >= durationS_ ? 100 : (elapsed * 100UL) / durationS_;
    }
    if (sampleCount_ == 0) return;
    float n = (float)sampleCount_;
    target["avgScore"] = sumScore_ / n;
    target["avgQuality"] = sumQuality_ / n;
    target["avgDeviation"] = sumDev_ / n;
    target["minScore"] = minScore_;
    target["maxScore"] = maxScore_;
    target["minQuality"] = minQuality_;
    target["maxQuality"] = maxQuality_;
}

void MeasSession::appendExportJson(JsonObject target) const {
    appendStatusJson(target);
    target["scoreVersion"] = RFMON_SCORE_VERSION;
    target["fwVersion"] = FW_VERSION;
    target["board"] = RFMON_BOARD_ID;
    JsonObject prof = target["sessionProfile"].to<JsonObject>();
    prof["id"] = profileId_;
    prof["title"] = profileTitle_;
    prof["durationS"] = durationS_;
    prof["wifiScanIntervalS"] = wifiIntervalS_;
    prof["bleScanIntervalS"] = bleIntervalS_;
    prof["bleEnabled"] = bleEnabled_;
    prof["characterization"] = characterization_;

    JsonArray ph = target["phases"].to<JsonArray>();
    for (uint8_t i = 0; i < phaseCount_; i++) {
        appendPhaseJson(ph.add<JsonObject>(), phases_[i]);
    }

    JsonArray arr = target["samples"].to<JsonArray>();
    for (size_t i = 0; i < ringCount_; i++) {
        size_t idx = (head_ + kMaxSamples - ringCount_ + i) % kMaxSamples;
        const Sample& s = ring_[idx];
        JsonObject o = arr.add<JsonObject>();
        o["ts"] = s.tsUptime;
        o["tsUptime"] = s.tsUptime;
        if (s.tsEpoch) o["tsEpoch"] = s.tsEpoch;
        o["score"] = s.score;
        o["quality"] = s.quality;
        o["deviation"] = s.deviation;
        o["apCount"] = s.apCount;
        o["bleCount"] = s.bleCount;
        o["pressure"] = s.pressure;
        o["occupancy"] = s.occupancy;
        o["density"] = s.density;
        o["jitter"] = s.jitter;
        o["stability"] = s.stability;
        o["connected"] = (s.flags & 0x01) != 0;
        o["ownRssi"] = s.ownRssi;
        o["radioDuty"] = s.radioDuty;
        if (s.phase[0]) o["phase"] = s.phase;
    }
}
