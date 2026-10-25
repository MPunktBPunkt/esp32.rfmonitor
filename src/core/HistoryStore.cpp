#include "HistoryStore.h"
#include "core/HwCaps.h"
#include <string.h>

void HistoryStore::begin() {
    capacity_ = HwCaps::historyCapacity();
    if (capacity_ < 16) capacity_ = 16;
    for (uint8_t i = 0; i < METRIC_COUNT; i++) {
        series_[i].head = 0;
        series_[i].count = 0;
        series_[i].values = (float*)HwCaps::alloc(capacity_ * sizeof(float), true);
        series_[i].ts = (uint32_t*)HwCaps::alloc(capacity_ * sizeof(uint32_t), true);
        if (series_[i].values) memset(series_[i].values, 0, capacity_ * sizeof(float));
        if (series_[i].ts) memset(series_[i].ts, 0, capacity_ * sizeof(uint32_t));
    }
}

void HistoryStore::push(Metric metric, float value) {
    if (metric >= METRIC_COUNT || !series_[metric].values || capacity_ == 0) return;
    Series& s = series_[metric];
    s.values[s.head] = value;
    s.ts[s.head] = millis() / 1000UL;
    s.head = (s.head + 1) % capacity_;
    if (s.count < capacity_) s.count++;
}

size_t HistoryStore::size(Metric metric) const {
    if (metric >= METRIC_COUNT) return 0;
    return series_[metric].count;
}

float HistoryStore::last(Metric metric, float fallback) const {
    if (metric >= METRIC_COUNT || series_[metric].count == 0 || !series_[metric].values) return fallback;
    size_t idx = (series_[metric].head + capacity_ - 1) % capacity_;
    return series_[metric].values[idx];
}

void HistoryStore::toJson(Metric metric, JsonArray arr, size_t maxPoints) const {
    if (metric >= METRIC_COUNT || !series_[metric].values || capacity_ == 0) return;
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
}
