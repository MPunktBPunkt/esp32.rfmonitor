#include "HistoryStore.h"
#include "core/HwCaps.h"
#include "core/TimeBase.h"
#include <string.h>
#include <math.h>

const char* HistoryStore::metricName(Metric m) {
    switch (m) {
        case RSSI: return "rssi";
        case RF_SCORE: return "rf_score";
        case AP_COUNT: return "ap_count";
        case BLE_COUNT: return "ble_count";
        case DATA_QUALITY: return "data_quality";
        case CHANNEL_PRESSURE: return "channel_pressure";
        default: return "unknown";
    }
}

const char* HistoryStore::resName(Res r) {
    switch (r) {
        case M1: return "1m";
        case M15: return "15m";
        default: return "raw";
    }
}

void HistoryStore::allocAgg(Agg& a, size_t cap) {
    a.cap = cap;
    a.head = a.count = 0;
    a.openN = 0;
    a.openStartS = 0;
    if (cap == 0) return;
    a.avg = (float*)HwCaps::alloc(cap * sizeof(float), true);
    a.mn = (float*)HwCaps::alloc(cap * sizeof(float), true);
    a.mx = (float*)HwCaps::alloc(cap * sizeof(float), true);
    a.ts = (uint32_t*)HwCaps::alloc(cap * sizeof(uint32_t), true);
    if (a.avg) memset(a.avg, 0, cap * sizeof(float));
    if (a.mn) memset(a.mn, 0, cap * sizeof(float));
    if (a.mx) memset(a.mx, 0, cap * sizeof(float));
    if (a.ts) memset(a.ts, 0, cap * sizeof(uint32_t));
}

void HistoryStore::begin() {
    capacity_ = HwCaps::historyCapacity();
    if (capacity_ < 16) capacity_ = 16;

    // Coarse rings: Mini keeps ~2h @ 1m and ~24h @ 15m; S3/PSRAM larger.
    size_t m1Cap = HwCaps::psram() ? 180 : 90;
    size_t m15Cap = HwCaps::psram() ? 192 : 96;
    chCap_ = HwCaps::psram() ? 120 : 48;

    for (uint8_t i = 0; i < METRIC_COUNT; i++) {
        series_[i].head = 0;
        series_[i].count = 0;
        series_[i].values = (float*)HwCaps::alloc(capacity_ * sizeof(float), true);
        series_[i].ts = (uint32_t*)HwCaps::alloc(capacity_ * sizeof(uint32_t), true);
        if (series_[i].values) memset(series_[i].values, 0, capacity_ * sizeof(float));
        if (series_[i].ts) memset(series_[i].ts, 0, capacity_ * sizeof(uint32_t));
        allocAgg(m1_[i], m1Cap);
        allocAgg(m15_[i], m15Cap);
    }

    chSnaps_ = (ChSnap*)HwCaps::alloc(chCap_ * sizeof(ChSnap), true);
    if (chSnaps_) memset(chSnaps_, 0, chCap_ * sizeof(ChSnap));
    chHead_ = chCount_ = 0;
    lastChSnapMs_ = 0;
}

void HistoryStore::closeAgg(Agg& a, uint32_t closeTs) {
    if (!a.avg || a.cap == 0 || a.openN == 0) {
        a.openN = 0;
        return;
    }
    float avg = (float)(a.sum / (double)a.openN);
    a.avg[a.head] = avg;
    a.mn[a.head] = a.openMin;
    a.mx[a.head] = a.openMax;
    a.ts[a.head] = closeTs ? closeTs : a.openStartS;
    a.head = (a.head + 1) % a.cap;
    if (a.count < a.cap) a.count++;
    a.openN = 0;
    a.sum = 0;
}

void HistoryStore::feedAgg(Agg& a, float value, uint32_t nowS, uint32_t bucketS,
                           Agg* promoteTo, uint32_t promoteBucketS) {
    if (!a.avg || a.cap == 0) return;
    if (a.openN == 0) {
        a.openStartS = nowS - (nowS % bucketS);
        a.sum = value;
        a.openMin = a.openMax = value;
        a.openN = 1;
        return;
    }
    uint32_t bucket = nowS - (nowS % bucketS);
    if (bucket != a.openStartS) {
        float closedAvg = (float)(a.sum / (double)a.openN);
        uint32_t closedTs = a.openStartS + bucketS;
        closeAgg(a, closedTs);
        if (promoteTo) {
            feedAgg(*promoteTo, closedAvg, closedTs, promoteBucketS, nullptr, 0);
        }
        a.openStartS = bucket;
        a.sum = value;
        a.openMin = a.openMax = value;
        a.openN = 1;
        return;
    }
    a.sum += value;
    if (value < a.openMin) a.openMin = value;
    if (value > a.openMax) a.openMax = value;
    a.openN++;
}

void HistoryStore::push(Metric metric, float value) {
    if (metric >= METRIC_COUNT || !series_[metric].values || capacity_ == 0) return;
    Series& s = series_[metric];
    uint32_t nowS = TimeBase::uptimeS();
    s.values[s.head] = value;
    s.ts[s.head] = nowS;
    s.head = (s.head + 1) % capacity_;
    if (s.count < capacity_) s.count++;

    feedAgg(m1_[metric], value, nowS, 60, &m15_[metric], 900);
}

void HistoryStore::pushChannels(const uint8_t ch[14]) {
    if (!chSnaps_ || chCap_ == 0 || !ch) return;
    unsigned long nowMs = millis();
    if (lastChSnapMs_ != 0 && nowMs - lastChSnapMs_ < kChSnapIntervalMs) return;
    lastChSnapMs_ = nowMs;

    ChSnap& snap = chSnaps_[chHead_];
    snap.ts = TimeBase::uptimeS();
    uint8_t peak = 0;
    for (uint8_t i = 0; i < 14; i++) {
        snap.ch[i] = ch[i];
        if (ch[i] > peak) peak = ch[i];
    }
    snap.peak = peak;
    chHead_ = (chHead_ + 1) % chCap_;
    if (chCount_ < chCap_) chCount_++;

    push(CHANNEL_PRESSURE, (float)(peak >= 8 ? 80 : peak * 10));
}

size_t HistoryStore::capacity(Res res) const {
    if (res == M1) return m1_[0].cap;
    if (res == M15) return m15_[0].cap;
    return capacity_;
}

size_t HistoryStore::size(Metric metric, Res res) const {
    if (metric >= METRIC_COUNT) return 0;
    if (res == M1) return m1_[metric].count;
    if (res == M15) return m15_[metric].count;
    return series_[metric].count;
}

float HistoryStore::last(Metric metric, float fallback) const {
    if (metric >= METRIC_COUNT || series_[metric].count == 0 || !series_[metric].values) return fallback;
    size_t idx = (series_[metric].head + capacity_ - 1) % capacity_;
    return series_[metric].values[idx];
}

void HistoryStore::toJson(Metric metric, JsonArray arr, size_t maxPoints, Res res) const {
    if (metric >= METRIC_COUNT) return;

    if (res == RAW) {
        if (!series_[metric].values || capacity_ == 0) return;
        const Series& s = series_[metric];
        if (maxPoints == 0) maxPoints = capacity_;
        size_t n = s.count < maxPoints ? s.count : maxPoints;
        size_t start = s.count > n ? s.count - n : 0;
        for (size_t i = start; i < s.count; i++) {
            size_t idx = (s.head + capacity_ - s.count + i) % capacity_;
            JsonObject o = arr.add<JsonObject>();
            o["ts"] = s.ts[idx];
            o["v"] = s.values[idx];
        }
        return;
    }

    const Agg& a = (res == M1) ? m1_[metric] : m15_[metric];
    if (!a.avg || a.cap == 0) return;
    if (maxPoints == 0) maxPoints = a.cap;
    size_t n = a.count < maxPoints ? a.count : maxPoints;
    size_t start = a.count > n ? a.count - n : 0;
    for (size_t i = start; i < a.count; i++) {
        size_t idx = (a.head + a.cap - a.count + i) % a.cap;
        JsonObject o = arr.add<JsonObject>();
        o["ts"] = a.ts[idx];
        o["v"] = a.avg[idx];
        o["min"] = a.mn[idx];
        o["max"] = a.mx[idx];
    }
}

void HistoryStore::channelsToJson(JsonArray arr, size_t maxPoints) const {
    if (!chSnaps_ || chCap_ == 0) return;
    if (maxPoints == 0) maxPoints = chCap_;
    size_t n = chCount_ < maxPoints ? chCount_ : maxPoints;
    size_t start = chCount_ > n ? chCount_ - n : 0;
    for (size_t i = start; i < chCount_; i++) {
        size_t idx = (chHead_ + chCap_ - chCount_ + i) % chCap_;
        const ChSnap& s = chSnaps_[idx];
        JsonObject o = arr.add<JsonObject>();
        o["ts"] = s.ts;
        o["peak"] = s.peak;
        JsonArray ch = o["ch"].to<JsonArray>();
        for (uint8_t c = 0; c < 14; c++) ch.add(s.ch[c]);
    }
}

void HistoryStore::appendMetaJson(JsonObject target) const {
    target["rawCapacity"] = (uint32_t)capacity_;
    target["m1Capacity"] = (uint32_t)m1_[0].cap;
    target["m15Capacity"] = (uint32_t)m15_[0].cap;
    target["channelSnapCapacity"] = (uint32_t)chCap_;
    target["channelSnapCount"] = (uint32_t)chCount_;
    target["channelSnapIntervalS"] = (uint32_t)(kChSnapIntervalMs / 1000UL);
    JsonObject sz = target["size"].to<JsonObject>();
    for (uint8_t i = 0; i < METRIC_COUNT; i++) {
        JsonObject m = sz[metricName((Metric)i)].to<JsonObject>();
        m["raw"] = (uint32_t)series_[i].count;
        m["1m"] = (uint32_t)m1_[i].count;
        m["15m"] = (uint32_t)m15_[i].count;
    }
}
