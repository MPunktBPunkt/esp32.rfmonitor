#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "BuildFlags.h"

// Runtime hardware capabilities. Mini D1: dual-core, usually no PSRAM.
// ESP32-S3: dual-core + optional PSRAM (enabled via BOARD_HAS_PSRAM).
class HwCaps {
public:
    static void begin();

    static bool psram() { return psram_; }
    static uint8_t cores() { return cores_; }
    static bool dualCore() { return cores_ >= 2 && RFMON_USE_DUAL_CORE; }
    static size_t psramFree() { return psramFree_; }
    static size_t psramSize() { return psramSize_; }
    static size_t historyCapacity() { return historyCap_; }
    static size_t eventCapacity() { return eventCap_; }
    static size_t maxAps() { return maxAps_; }
    static size_t maxBle() { return maxBle_; }
    static bool workerRunning() { return workerRunning_; }
    static void setWorkerRunning(bool v) { workerRunning_ = v; }

    // Prefer PSRAM when present; fall back to internal heap.
    static void* alloc(size_t bytes, bool preferPsram = true);
    static void release(void* ptr);

    static void appendJson(JsonObject target);

private:
    static bool psram_;
    static uint8_t cores_;
    static size_t psramFree_;
    static size_t psramSize_;
    static size_t historyCap_;
    static size_t eventCap_;
    static size_t maxAps_;
    static size_t maxBle_;
    static bool workerRunning_;
};
