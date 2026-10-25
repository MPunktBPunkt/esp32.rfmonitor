#include "HwCaps.h"
#include <esp_heap_caps.h>
#include <esp_system.h>

bool HwCaps::psram_ = false;
uint8_t HwCaps::cores_ = 1;
size_t HwCaps::psramFree_ = 0;
size_t HwCaps::psramSize_ = 0;
size_t HwCaps::historyCap_ = RFMON_HISTORY_SIZE;
size_t HwCaps::eventCap_ = RFMON_EVENT_SIZE;
size_t HwCaps::maxAps_ = RFMON_MAX_APS;
size_t HwCaps::maxBle_ = RFMON_MAX_BLE;
bool HwCaps::workerRunning_ = false;

void HwCaps::begin() {
    cores_ = (uint8_t)ESP.getChipCores();
    // Prefer heap_caps: on some S3 Arduino builds getPsramSize()/getFreePsram() report 0
    // even when SPIRAM is mapped and psramFound() is true.
    size_t spiramTotal = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    size_t spiramFree = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
#if defined(BOARD_HAS_PSRAM) || defined(CONFIG_SPIRAM)
    psram_ = psramFound() || spiramTotal > 0;
#else
    psram_ = spiramTotal > 0;
#endif
    if (psram_) {
        psramSize_ = spiramTotal ? spiramTotal : (size_t)ESP.getPsramSize();
        psramFree_ = spiramFree ? spiramFree : (size_t)ESP.getFreePsram();
        historyCap_ = RFMON_HISTORY_SIZE_PSRAM;
        eventCap_ = RFMON_EVENT_SIZE_PSRAM;
        maxAps_ = RFMON_MAX_APS_PSRAM;
        maxBle_ = RFMON_MAX_BLE_PSRAM;
    } else {
        historyCap_ = RFMON_HISTORY_SIZE;
        eventCap_ = RFMON_EVENT_SIZE;
        maxAps_ = RFMON_MAX_APS;
        maxBle_ = RFMON_MAX_BLE;
    }
    Serial.printf("[HW] cores=%u psram=%s size=%u free=%u hist=%u aps=%u ble=%u dual=%s\n",
                  (unsigned)cores_,
                  psram_ ? "yes" : "no",
                  (unsigned)psramSize_,
                  (unsigned)psramFree_,
                  (unsigned)historyCap_,
                  (unsigned)maxAps_,
                  (unsigned)maxBle_,
                  dualCore() ? "yes" : "no");
}

void* HwCaps::alloc(size_t bytes, bool preferPsram) {
    if (!bytes) return nullptr;
    void* p = nullptr;
    if (preferPsram && psram_) {
        p = heap_caps_malloc(bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    }
    if (!p) p = malloc(bytes);
    return p;
}

void HwCaps::release(void* ptr) {
    if (ptr) free(ptr);
}

void HwCaps::appendJson(JsonObject target) {
    target["cores"] = cores_;
    target["dualCore"] = dualCore();
    target["workerRunning"] = workerRunning_;
    target["psram"] = psram_;
    size_t freeNow = psram_ ? heap_caps_get_free_size(MALLOC_CAP_SPIRAM) : 0;
    if (psram_ && freeNow == 0) freeNow = (size_t)ESP.getFreePsram();
    target["psramSize"] = (uint32_t)psramSize_;
    target["psramFree"] = (uint32_t)freeNow;
    target["heapFree"] = ESP.getFreeHeap();
    target["historyCapacity"] = (uint32_t)historyCap_;
    target["eventCapacity"] = (uint32_t)eventCap_;
    target["maxAps"] = (uint32_t)maxAps_;
    target["maxBle"] = (uint32_t)maxBle_;
}
