#include "RfAnalyzer.h"
#include "core/HistoryStore.h"
#include <math.h>
#include <ArduinoJson.h>

void RfAnalyzer::begin(ConfigStore* cfg, EventLog* events, HistoryStore* history, RfScanner* scanner, BleTracker* ble) {
    cfg_ = cfg;
    events_ = events;
    history_ = history;
    scanner_ = scanner;
    ble_ = ble;
    lastWifi_ = WiFi.status();
    lastRssi_ = (int8_t)WiFi.RSSI();
}

void RfAnalyzer::loop() {
    wl_status_t st = WiFi.status();
    if (lastWifi_ == WL_CONNECTED && st != WL_CONNECTED) {
        disconnects_++;
        if (stability_ > 8) stability_ -= 8;
        if (events_) events_->warn("system.reconnect", "WLAN-Verbindung verloren", "");
        if (events_) events_->warn("wifi.rssi_drop", "Eigene WLAN-Verbindung kritisch", "");
    }
    if (lastWifi_ != WL_CONNECTED && st == WL_CONNECTED && events_) {
        events_->info("system.reconnect", "WLAN wieder verbunden", WiFi.SSID().c_str());
    }
    if (st == WL_CONNECTED && stability_ < 100 && millis() % 10000UL < 50UL) {
        stability_++;
    }
    lastWifi_ = st;

    if (millis() - lastCompute_ >= 4000UL) {
        lastCompute_ = millis();
        recompute();
    }
}

void RfAnalyzer::noteHub(bool ok) {
    if (hubWasOk_ && !ok && events_) {
        events_->warn("system.hub_offline", "ESP-Hub nicht erreichbar", "");
    }
    hubWasOk_ = ok;
}

void RfAnalyzer::recompute() {
    int8_t rssi = (int8_t)WiFi.RSSI();
    rssiWindow_[rssiHead_] = rssi;
    rssiHead_ = (rssiHead_ + 1) % 12;
    if (rssiCount_ < 12) rssiCount_++;

    float sum = 0;
    int8_t mn = 127, mx = -127;
    for (uint8_t i = 0; i < rssiCount_; i++) {
        sum += rssiWindow_[i];
        if (rssiWindow_[i] < mn) mn = rssiWindow_[i];
        if (rssiWindow_[i] > mx) mx = rssiWindow_[i];
    }
    rssiAvg_ = rssiCount_ ? sum / rssiCount_ : rssi;
    rssiJitter_ = (float)(mx - mn);
    rssiMin_ = mn == 127 ? rssi : mn;
    rssiMax_ = mx == -127 ? rssi : mx;

    uint8_t ch[14] = {0};
    uint8_t aps = 0;
    if (scanner_) {
        scanner_->channelOccupancy(ch);
        aps = scanner_->apCount();
        if (scanner_->averageRssi() != 0) {
            rssiAvg_ = 0.6f * rssiAvg_ + 0.4f * scanner_->averageRssi();
        }
    }
    uint8_t maxCh = 0;
    uint8_t occupied = 0;
    for (uint8_t i = 0; i < 14; i++) {
        if (ch[i] > maxCh) maxCh = ch[i];
        if (ch[i] > 0) occupied++;
    }
    congestion_ = maxCh >= 8 ? 80 : (uint8_t)(maxCh * 10);
    channelLoad_ = occupied >= 13 ? 100 : (uint8_t)(occupied * 8);
    density_ = aps >= 20 ? 80 : (uint8_t)(aps * 4);

    int score = 100;
    if (rssi < -55) score -= (int)((-55 - rssi) * 1.2f);
    if (rssiJitter_ > 8) score -= (int)((rssiJitter_ - 8) * 2);
    score -= congestion_ / 4;
    score -= density_ / 6;
    score -= (100 - stability_) / 3;
    if (WiFi.status() != WL_CONNECTED) score -= 25;
    if (score < 0) score = 0;
    if (score > 100) score = 100;
    uint8_t prev = score_;
    score_ = (uint8_t)score;

    if (history_) {
        history_->push(HistoryStore::RSSI, (float)rssi);
        history_->push(HistoryStore::RF_SCORE, (float)score_);
    }

    int drop = cfg_ ? cfg_->rssiWarnDrop : 12;
    if (lastRssi_ != 0 && rssi <= lastRssi_ - drop && events_) {
        events_->warn("wifi.rssi_drop", "RSSI deutlich gefallen", String(rssi).c_str());
    }
    if (lastRssi_ != 0 && lastRssi_ <= -70 && rssi >= lastRssi_ + drop && events_) {
        events_->info("wifi.rssi_recover", "RSSI erholt", String(rssi).c_str());
    }
    lastRssi_ = rssi;

    uint8_t warnScore = cfg_ ? cfg_->rfScoreWarn : 40;
    if (prev >= warnScore && score_ < warnScore && events_) {
        events_->warn("rf.score_low", "RF-Score schlecht", String(score_).c_str());
    }
    uint8_t congWarn = cfg_ ? cfg_->congestionWarn : 70;
    if (congestion_ >= congWarn && events_ && millis() / 4000UL % 8 == 0) {
        events_->warn("rf.congestion_high", "Kanalueberlastung", String(congestion_).c_str());
    }
}

void RfAnalyzer::appendStatusJson(JsonObject target) const {
    target["rfScore"] = score_;
    target["wifiRssi"] = WiFi.RSSI();
    target["wifiRssiAvg"] = rssiAvg_;
    target["wifiRssiMin"] = rssiMin_;
    target["wifiRssiMax"] = rssiMax_;
    target["wifiRssiJitter"] = rssiJitter_;
    target["channelLoad"] = channelLoad_;
    target["channelCongestion"] = congestion_;
    target["apDensity"] = density_;
    target["stability"] = stability_;
    target["disconnects"] = disconnects_;
    if (scanner_) {
        target["apCount"] = scanner_->apCount();
        target["knownApCount"] = scanner_->knownCount();
        target["unknownApCount"] = scanner_->unknownCount();
        target["strongestRssi"] = scanner_->strongestRssi();
        target["scanFailCount"] = scanner_->failCount();
    }
    if (ble_) {
        target["bleCount"] = ble_->deviceCount();
        target["beaconCount"] = ble_->beaconCount();
        target["newBleCount"] = ble_->newCount();
    }
}

void RfAnalyzer::appendIoValues(JsonObject ios) const {
    auto add = [&](const char* key, float value, const char* unit) {
        JsonObject o = ios[key].to<JsonObject>();
        o["type"] = "sensor";
        o["value"] = value;
        o["unit"] = unit;
    };
    add("wifi_rssi", (float)WiFi.RSSI(), "dBm");
    add("wifi_rssi_avg", rssiAvg_, "dBm");
    add("wifi_rssi_jitter", rssiJitter_, "dB");
    add("rf_score", (float)score_, "");
    add("ap_count", scanner_ ? (float)scanner_->apCount() : 0, "");
    add("ble_count", ble_ ? (float)ble_->deviceCount() : 0, "");
    add("beacon_count", ble_ ? (float)ble_->beaconCount() : 0, "");
    add("channel_load", (float)channelLoad_, "");
    add("scan_fail_count", scanner_ ? (float)scanner_->failCount() : 0, "");
}
