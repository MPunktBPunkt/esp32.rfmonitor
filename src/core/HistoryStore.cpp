#include "HistoryStore.h"

void HistoryStore::begin() {
    for (uint8_t i = 0; i < METRIC_COUNT; i++) {
        series_[i].head = 0;
        series_[i].count = 0;
    }
}

void HistoryStore::push(Metric metric, float value) {
    if (metric >= METRIC_COUNT) return;
    Series& s = series_[metric];
    s.values[s.head] = value;
    s.ts[s.head] = millis() / 1000UL;
    s.head = (s.head + 1) % RFMON_HISTORY_SIZE;
    if (s.count < RFMON_HISTORY_SIZE) s.count++;
}

size_t HistoryStore::size(Metric metric) const {
    if (metric >= METRIC_COUNT) return 0;
    return series_[metric].count;
}

float HistoryStore::last(Metric metric, float fallback) const {
    if (metric >= METRIC_COUNT || series_[metric].count == 0) return fallback;
    size_t idx = (series_[metric].head + RFMON_HISTORY_SIZE - 1) % RFMON_HISTORY_SIZE;
    return series_[metric].values[idx];
}

void HistoryStore::toJson(Metric metric, JsonArray arr, size_t maxPoints) const {
    if (metric >= METRIC_COUNT) return;
    const Series& s = series_[metric];
    size_t n = s.count < maxPoints ? s.count : maxPoints;
    size_t start = s.count > n ? s.count - n : 0;
    for (size_t i = start; i < s.count; i++) {
        size_t idx = (s.head + RFMON_HISTORY_SIZE - s.count + i) % RFMON_HISTORY_SIZE;
        JsonObject o = arr.add<JsonObject>();
        o["ts"] = s.ts[idx];
        o["v"] = s.values[idx];
    }
}
