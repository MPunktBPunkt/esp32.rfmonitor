#include "RfScanner.h"
#include <WiFi.h>
#include <string.h>
#include <ArduinoJson.h>

void RfScanner::begin(ConfigStore* cfg, EventLog* events, HistoryStore* history) {
    cfg_ = cfg;
    events_ = events;
    history_ = history;
    memset(aps_, 0, sizeof(aps_));
    memset(prevBssid_, 0, sizeof(prevBssid_));
    lastScanStart_ = millis() - (unsigned long)cfg_->wifiScanIntervalS * 1000UL + 3000UL;
}

void RfScanner::pause() {
    if (scanning_ && WiFi.scanComplete() == WIFI_SCAN_RUNNING) {
        WiFi.scanDelete();
        scanning_ = false;
    }
}

bool RfScanner::isKnownSsid(const String& ssid) const {
    if (ssid.length() == 0) return false;
    String connected = WiFi.SSID();
    if (connected.length() && ssid.equalsIgnoreCase(connected)) return true;
    if (!cfg_ || cfg_->knownSsids.length() == 0) return false;
    String list = cfg_->knownSsids;
    list.toLowerCase();
    String needle = ssid;
    needle.toLowerCase();
    int start = 0;
    while (start < (int)list.length()) {
        int comma = list.indexOf(',', start);
        if (comma < 0) comma = list.length();
        String item = list.substring(start, comma);
        item.trim();
        if (item.length() && item == needle) return true;
        start = comma + 1;
    }
    return false;
}

bool RfScanner::requestScan() {
    if (scanning_) return false;
    if (WiFi.scanComplete() == WIFI_SCAN_RUNNING) return false;
    int16_t rc = WiFi.scanNetworks(true, true);
    scanning_ = (rc == WIFI_SCAN_RUNNING || rc >= 0);
    if (scanning_) lastScanStart_ = millis();
    if (rc == WIFI_SCAN_FAILED) {
        failCount_++;
        if (events_) events_->warn("wifi.scan_error", "WLAN-Scan fehlgeschlagen", "");
        return false;
    }
    return scanning_;
}

bool RfScanner::due() const {
    unsigned long interval = (unsigned long)(cfg_ ? cfg_->wifiScanIntervalS : 20) * 1000UL;
    if (interval < 8000UL) interval = 8000UL;
    return !scanning_ && (millis() - lastScanStart_ >= interval);
}

void RfScanner::loop(bool mayStart) {
    int16_t n = WiFi.scanComplete();
    if (n == WIFI_SCAN_RUNNING) {
        scanning_ = true;
        return;
    }
    if (scanning_ && n >= 0) {
        processScan(n);
        WiFi.scanDelete();
        scanning_ = false;
        return;
    }
    if (n == WIFI_SCAN_FAILED && scanning_) {
        scanning_ = false;
        failCount_++;
        if (events_) events_->warn("wifi.scan_error", "WLAN-Scan fehlgeschlagen", "");
    }
    if (mayStart && due()) requestScan();
}

void RfScanner::emitDiff() {
    if (!events_) return;
    for (uint8_t i = 0; i < apCount_; i++) {
        bool found = false;
        for (uint8_t p = 0; p < prevCount_; p++) {
            if (strcmp(aps_[i].bssid, prevBssid_[p]) == 0) {
                found = true;
                break;
            }
        }
        if (!found && prevCount_ > 0) {
            String d = String(aps_[i].ssid) + " " + aps_[i].bssid;
            events_->info("wifi.new_ap", "Neues WLAN erkannt", d.c_str());
        }
    }
    for (uint8_t p = 0; p < prevCount_; p++) {
        bool found = false;
        for (uint8_t i = 0; i < apCount_; i++) {
            if (strcmp(aps_[i].bssid, prevBssid_[p]) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            events_->warn("wifi.ap_lost", "WLAN verschwunden", prevBssid_[p]);
        }
    }
}

void RfScanner::processScan(int16_t n) {
    apCount_ = 0;
    knownCount_ = 0;
    strongest_ = -127;
    float sum = 0;
    int16_t limit = n > RFMON_MAX_APS ? RFMON_MAX_APS : n;
    for (int16_t i = 0; i < limit; i++) {
        ApRecord& ap = aps_[apCount_];
        String ssid = WiFi.SSID(i);
        String bssid = WiFi.BSSIDstr(i);
        strncpy(ap.ssid, ssid.c_str(), sizeof(ap.ssid) - 1);
        ap.ssid[sizeof(ap.ssid) - 1] = 0;
        strncpy(ap.bssid, bssid.c_str(), sizeof(ap.bssid) - 1);
        ap.bssid[sizeof(ap.bssid) - 1] = 0;
        ap.rssi = (int8_t)WiFi.RSSI(i);
        ap.channel = (uint8_t)WiFi.channel(i);
        ap.enc = (uint8_t)WiFi.encryptionType(i);
        ap.hidden = ssid.length() == 0;
        ap.known = isKnownSsid(ssid);
        ap.lastSeen = millis() / 1000UL;
        if (ap.known) knownCount_++;
        if (ap.rssi > strongest_) strongest_ = ap.rssi;
        sum += ap.rssi;
        apCount_++;
    }
    avgRssi_ = apCount_ ? (sum / apCount_) : 0;
    lastScanTs_ = millis() / 1000UL;
    emitDiff();
    prevCount_ = apCount_;
    for (uint8_t i = 0; i < apCount_; i++) {
        strncpy(prevBssid_[i], aps_[i].bssid, 18);
        prevBssid_[i][17] = 0;
    }
    if (history_) history_->push(HistoryStore::AP_COUNT, (float)apCount_);
}

void RfScanner::channelOccupancy(uint8_t out[14]) const {
    memset(out, 0, 14);
    for (uint8_t i = 0; i < apCount_; i++) {
        uint8_t ch = aps_[i].channel;
        if (ch >= 1 && ch <= 14) out[ch - 1]++;
    }
}

void RfScanner::resultsToJson(JsonArray arr) const {
    for (uint8_t i = 0; i < apCount_; i++) {
        JsonObject o = arr.add<JsonObject>();
        o["ssid"] = aps_[i].ssid;
        o["bssid"] = aps_[i].bssid;
        o["rssi"] = aps_[i].rssi;
        o["channel"] = aps_[i].channel;
        o["enc"] = aps_[i].enc;
        o["hidden"] = aps_[i].hidden;
        o["known"] = aps_[i].known;
        o["lastSeen"] = aps_[i].lastSeen;
    }
}

void RfScanner::channelsToJson(JsonArray arr) const {
    uint8_t ch[14];
    channelOccupancy(ch);
    for (uint8_t i = 0; i < 14; i++) {
        JsonObject o = arr.add<JsonObject>();
        o["ch"] = i + 1;
        o["count"] = ch[i];
    }
}
