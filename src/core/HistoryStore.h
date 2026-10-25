#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "BuildFlags.h"

class HistoryStore {
public:
    enum Metric : uint8_t {
        RSSI = 0,
        RF_SCORE = 1,
        AP_COUNT = 2,
        BLE_COUNT = 3,
        DATA_QUALITY = 4,
        METRIC_COUNT = 5
    };

    void begin();
    void push(Metric metric, float value);
    size_t size(Metric metric) const;
    size_t capacity() const { return capacity_; }
    void toJson(Metric metric, JsonArray arr, size_t maxPoints = 0) const;
    float last(Metric metric, float fallback = 0) const;

private:
    struct Series {
        float* values = nullptr;
        uint32_t* ts = nullptr;
        size_t head = 0;
        size_t count = 0;
    };
    Series series_[METRIC_COUNT];
    size_t capacity_ = RFMON_HISTORY_SIZE;
};
