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
        METRIC_COUNT = 4
    };

    void begin();
    void push(Metric metric, float value);
    size_t size(Metric metric) const;
    void toJson(Metric metric, JsonArray arr, size_t maxPoints = RFMON_HISTORY_SIZE) const;
    float last(Metric metric, float fallback = 0) const;

private:
    struct Series {
        float values[RFMON_HISTORY_SIZE];
        uint32_t ts[RFMON_HISTORY_SIZE];
        size_t head = 0;
        size_t count = 0;
    };
    Series series_[METRIC_COUNT];
};
