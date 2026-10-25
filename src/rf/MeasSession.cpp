#include "MeasSession.h"
#include <string.h>
#include <math.h>

void MeasSession::begin() {
    active_ = false;
    sampleCount_ = 0;
    ringCount_ = 0;
    head_ = 0;
}

bool MeasSession::start(const char* label) {
    active_ = true;
    startS_ = millis() / 1000UL;
    endS_ = 0;
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
    memset(label_, 0, sizeof(label_));
    if (label && label[0]) strncpy(label_, label, sizeof(label_) - 1);
    return true;
}

bool MeasSession::stop() {
    if (!active_) return false;
    active_ = false;
    endS_ = millis() / 1000UL;
    return true;
}

void MeasSession::noteSample(uint8_t score, uint8_t quality, int8_t deviation,
                             uint8_t apCount, uint8_t bleCount, uint8_t pressure,
                             float ownRssi, float radioDuty, uint8_t eventLevel) {
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

    Sample& s = ring_[head_];
    s.ts = millis() / 1000UL;
    s.score = score;
    s.quality = quality;
    s.deviation = deviation;
    s.apCount = apCount;
    s.bleCount = bleCount;
    s.pressure = pressure;
    s.ownRssi = (int8_t)lroundf(ownRssi);
    s.radioDuty = radioDuty > 255 ? 255 : (uint8_t)lroundf(radioDuty);
    head_ = (head_ + 1) % kMaxSamples;
    if (ringCount_ < kMaxSamples) ringCount_++;
}

void MeasSession::appendStatusJson(JsonObject target) const {
    target["active"] = active_;
    target["label"] = label_;
    target["startS"] = startS_;
    target["endS"] = endS_;
    target["sampleCount"] = sampleCount_;
    target["ringCount"] = (uint32_t)ringCount_;
    target["warnEvents"] = warnEvents_;
    target["errorEvents"] = errorEvents_;
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
    JsonArray arr = target["samples"].to<JsonArray>();
    for (size_t i = 0; i < ringCount_; i++) {
        size_t idx = (head_ + kMaxSamples - ringCount_ + i) % kMaxSamples;
        const Sample& s = ring_[idx];
        JsonObject o = arr.add<JsonObject>();
        o["ts"] = s.ts;
        o["score"] = s.score;
        o["quality"] = s.quality;
        o["deviation"] = s.deviation;
        o["apCount"] = s.apCount;
        o["bleCount"] = s.bleCount;
        o["pressure"] = s.pressure;
        o["ownRssi"] = s.ownRssi;
        o["radioDuty"] = s.radioDuty;
    }
}
