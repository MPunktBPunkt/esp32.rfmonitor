#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "BuildFlags.h"

// Raw ring (~scan cadence) + 1-minute / 15-minute aggregates + channel snaps.
class HistoryStore {
public:
    enum Metric : uint8_t {
        RSSI = 0,
        RF_SCORE = 1,
        AP_COUNT = 2,
        BLE_COUNT = 3,
        DATA_QUALITY = 4,
        CHANNEL_PRESSURE = 5,
        METRIC_COUNT = 6
    };
    enum Res : uint8_t { RAW = 0, M1 = 1, M15 = 2 };

    void begin();
    void push(Metric metric, float value);
    void pushChannels(const uint8_t ch[14]);  // AP count per channel 1..14
    size_t size(Metric metric, Res res = RAW) const;
    size_t capacity(Res res = RAW) const;
    size_t channelSnapCount() const { return chCount_; }
    size_t channelSnapCapacity() const { return chCap_; }
    void toJson(Metric metric, JsonArray arr, size_t maxPoints = 0, Res res = RAW) const;
    void channelsToJson(JsonArray arr, size_t maxPoints = 0) const;
    void appendMetaJson(JsonObject target) const;
    float last(Metric metric, float fallback = 0) const;
    static const char* metricName(Metric m);
    static const char* resName(Res r);

private:
    struct Series {
        float* values = nullptr;
        uint32_t* ts = nullptr;
        size_t head = 0;
        size_t count = 0;
    };
    struct Agg {
        float* avg = nullptr;
        float* mn = nullptr;
        float* mx = nullptr;
        uint32_t* ts = nullptr;
        size_t cap = 0;
        size_t head = 0;
        size_t count = 0;
        double sum = 0;
        float openMin = 0;
        float openMax = 0;
        uint16_t openN = 0;
        uint32_t openStartS = 0;
    };
    struct ChSnap {
        uint32_t ts;
        uint8_t ch[14];
        uint8_t peak;  // max APs on any channel
    };

    void feedAgg(Agg& a, float value, uint32_t nowS, uint32_t bucketS, Agg* promoteTo, uint32_t promoteBucketS);
    void closeAgg(Agg& a, uint32_t closeTs);
    void allocAgg(Agg& a, size_t cap);

    Series series_[METRIC_COUNT];
    Agg m1_[METRIC_COUNT];
    Agg m15_[METRIC_COUNT];
    size_t capacity_ = RFMON_HISTORY_SIZE;

    ChSnap* chSnaps_ = nullptr;
    size_t chCap_ = 0;
    size_t chHead_ = 0;
    size_t chCount_ = 0;
    unsigned long lastChSnapMs_ = 0;
    static constexpr uint32_t kChSnapIntervalMs = 30000UL;
};
