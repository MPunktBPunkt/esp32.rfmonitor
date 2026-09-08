#include "BleTracker.h"
#include <string.h>
#include <ArduinoJson.h>
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

#if RFMON_ENABLE_BLE
#include <NimBLEDevice.h>

static BleTracker* g_ble = nullptr;

class RfAdvCallbacks : public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* dev) override {
        if (!g_ble || !dev) return;
        uint8_t kind = 0;
        uint16_t mfgId = 0;
        if (dev->haveManufacturerData()) {
            std::string md = dev->getManufacturerData();
            if (md.size() >= 2) {
                mfgId = (uint16_t)((uint8_t)md[1] << 8 | (uint8_t)md[0]);
                if (mfgId == 0x004C && md.size() >= 4 && (uint8_t)md[2] == 0x02 && (uint8_t)md[3] == 0x15) {
                    kind = 1;
                } else if (md.size() >= 4 && (uint8_t)md[2] == 0xBE && (uint8_t)md[3] == 0xAC) {
                    kind = 3;
                }
            }
        }
        if (dev->isAdvertisingService(NimBLEUUID((uint16_t)0xFEAA))) kind = 2;
        String name = dev->haveName() ? String(dev->getName().c_str()) : "";
        if (kind == 0 && name.indexOf("eacon") >= 0) kind = 3;
        g_ble->ingest(dev->getAddress().toString().c_str(), name.c_str(),
                      (int8_t)dev->getRSSI(), mfgId, kind);
    }
};

static RfAdvCallbacks g_advCb;
#endif

void BleTracker::begin(ConfigStore* cfg, EventLog* events, HistoryStore* history) {
    cfg_ = cfg;
    events_ = events;
    history_ = history;
    memset(devices_, 0, sizeof(devices_));
    lastStart_ = millis();
#if RFMON_ENABLE_BLE
    g_ble = this;
    if (cfg_ && !cfg_->bleEnabled) return;
    NimBLEDevice::init("");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    NimBLEScan* scan = NimBLEDevice::getScan();
    scan->setAdvertisedDeviceCallbacks(&g_advCb, false);
    scan->setActiveScan(true);
    scan->setInterval(160);
    scan->setWindow(80);
    ready_ = true;
#endif
}

bool BleTracker::isKnown(const char* addr, const char* name) const {
    if (!cfg_ || cfg_->knownBle.length() == 0) return false;
    String list = cfg_->knownBle;
    list.toLowerCase();
    String a = addr ? addr : "";
    String n = name ? name : "";
    a.toLowerCase();
    n.toLowerCase();
    int start = 0;
    while (start < (int)list.length()) {
        int comma = list.indexOf(',', start);
        if (comma < 0) comma = list.length();
        String item = list.substring(start, comma);
        item.trim();
        if (item.length() && (item == a || (n.length() && item == n))) return true;
        start = comma + 1;
    }
    return false;
}

int BleTracker::findIndex(const char* addr) const {
    for (uint8_t i = 0; i < count_; i++) {
        if (strcasecmp(devices_[i].addr, addr) == 0) return i;
    }
    return -1;
}

void BleTracker::ingest(const char* addr, const char* name, int8_t rssi, uint16_t mfgId, uint8_t kind) {
    if (!addr || !addr[0]) return;
    uint32_t now = millis() / 1000UL;
    bool known = isKnown(addr, name);
    portENTER_CRITICAL(&mux_);
    int idx = findIndex(addr);
    if (idx < 0) {
        if (count_ >= RFMON_MAX_BLE) {
            uint8_t oldest = 0;
            uint32_t oldestTs = devices_[0].lastSeen;
            for (uint8_t i = 1; i < count_; i++) {
                if (devices_[i].lastSeen < oldestTs) {
                    oldestTs = devices_[i].lastSeen;
                    oldest = i;
                }
            }
            idx = oldest;
        } else {
            idx = count_++;
        }
        BleRecord& nd = devices_[idx];
        memset(&nd, 0, sizeof(nd));
        strncpy(nd.addr, addr, sizeof(nd.addr) - 1);
        nd.firstSeen = now;
        nd.hits = 0;
        newCount_++;
        if (pendingCount_ < 8) {
            strncpy(pendingAddr_[pendingCount_], addr, 17);
            pendingKind_[pendingCount_] = kind;
            pendingCount_++;
        }
    }
    BleRecord& d = devices_[idx];
    if (name && name[0]) strncpy(d.name, name, sizeof(d.name) - 1);
    d.rssi = rssi;
    d.mfgId = mfgId;
    if (kind > d.kind) d.kind = kind;
    d.hits++;
    d.lastSeen = now;
    d.known = known;
    portEXIT_CRITICAL(&mux_);
}

void BleTracker::prune() {
    uint32_t now = millis() / 1000UL;
    uint8_t w = 0;
    beaconCount_ = 0;
    for (uint8_t i = 0; i < count_; i++) {
        if (now - devices_[i].lastSeen > 90) {
            if (events_) events_->info("ble.device_lost", "BLE-Geraet verloren", devices_[i].addr);
            continue;
        }
        if (w != i) devices_[w] = devices_[i];
        if (devices_[w].kind > 0) beaconCount_++;
        w++;
    }
    count_ = w;
}

bool BleTracker::startWindow() {
#if RFMON_ENABLE_BLE
    if (!ready_ || !cfg_ || !cfg_->bleEnabled) return false;
    if (scanning_) return false;
    NimBLEScan* scan = NimBLEDevice::getScan();
    if (scan->isScanning()) return false;
    newCount_ = 0;
    uint32_t dur = 4;
    if (!scan->start(dur, nullptr, false)) return false;
    scanning_ = true;
    windowStart_ = millis();
    lastStart_ = millis();
    return true;
#else
    return false;
#endif
}

void BleTracker::stop() {
#if RFMON_ENABLE_BLE
    if (!ready_) return;
    NimBLEScan* scan = NimBLEDevice::getScan();
    if (scan->isScanning()) scan->stop();
#endif
    scanning_ = false;
}

bool BleTracker::due() const {
    if (!ready_ || !cfg_ || !cfg_->bleEnabled) return false;
    unsigned long interval = (unsigned long)cfg_->bleScanIntervalS * 1000UL;
    if (interval < 5000UL) interval = 5000UL;
    return !scanning_ && (millis() - lastStart_ >= interval);
}

void BleTracker::loop(bool mayStart) {
#if RFMON_ENABLE_BLE
    if (!ready_ || !cfg_ || !cfg_->bleEnabled) return;
    char evAddr[8][18];
    uint8_t evKind[8];
    uint8_t evN = 0;
    portENTER_CRITICAL(&mux_);
    evN = pendingCount_;
    for (uint8_t i = 0; i < evN; i++) {
        strncpy(evAddr[i], pendingAddr_[i], 18);
        evKind[i] = pendingKind_[i];
    }
    pendingCount_ = 0;
    portEXIT_CRITICAL(&mux_);
    if (events_) {
        for (uint8_t i = 0; i < evN; i++) {
            if (evKind[i] > 0) events_->info("ble.new_beacon", "Neuer BLE-Beacon", evAddr[i]);
            else events_->info("ble.new_device", "Neues BLE-Geraet", evAddr[i]);
        }
    }
    NimBLEScan* scan = NimBLEDevice::getScan();
    if (scanning_ && !scan->isScanning()) {
        scanning_ = false;
        prune();
        if (history_) history_->push(HistoryStore::BLE_COUNT, (float)count_);
    }
    if (millis() - lastPrune_ >= 15000UL) {
        lastPrune_ = millis();
        prune();
    }
    if (mayStart && due()) startWindow();
#endif
}

void BleTracker::resultsToJson(JsonArray arr, bool beaconsOnly) const {
    for (uint8_t i = 0; i < count_; i++) {
        if (beaconsOnly && devices_[i].kind == 0) continue;
        JsonObject o = arr.add<JsonObject>();
        o["addr"] = devices_[i].addr;
        o["name"] = devices_[i].name;
        o["rssi"] = devices_[i].rssi;
        o["mfgId"] = devices_[i].mfgId;
        o["kind"] = devices_[i].kind;
        o["hits"] = devices_[i].hits;
        o["firstSeen"] = devices_[i].firstSeen;
        o["lastSeen"] = devices_[i].lastSeen;
        o["known"] = devices_[i].known;
        const char* label = "advertiser";
        if (devices_[i].kind == 1) label = "iBeacon";
        else if (devices_[i].kind == 2) label = "Eddystone";
        else if (devices_[i].kind == 3) label = "beacon";
        o["type"] = label;
    }
}
