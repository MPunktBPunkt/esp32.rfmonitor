#include "RfScanner.h"
#include "core/HwCaps.h"
#include <WiFi.h>
#include <string.h>
#include <ArduinoJson.h>

void RfScanner::begin(ConfigStore* cfg, EventLog* events, HistoryStore* history) {
    cfg_ = cfg;
    events_ = events;
    history_ = history;
    capacity_ = HwCaps::maxAps();
    if (capacity_ < 8) capacity_ = 8;
    aps_ = (ApRecord*)HwCaps::alloc(capacity_ * sizeof(ApRecord), true);
    if (aps_) memset(aps_, 0, capacity_ * sizeof(ApRecord));
    tracked_ = 0;
    lastScanStart_ = millis() - (unsigned long)cfg_->wifiScanIntervalS * 1000UL + 3000UL;
}

void RfScanner::pause() {
    if (scanning_ && WiFi.scanComplete() == WIFI_SCAN_RUNNING) {
        WiFi.scanDelete();
        scanning_ = false;
    }
}

const char* RfScanner::lifeName(ApLife life) {
    switch (life) {
        case ApLife::New: return "new";
        case ApLife::Persistent: return "persistent";
        case ApLife::Flapping: return "flapping";
        case ApLife::Disappeared: return "disappeared";
        default: return "unknown";
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

void RfScanner::noteFail() {
    failCount_++;
    lastScanOk_ = false;
    if (events_) {
        events_->warn("wifi.scan_error", "WLAN-Scan fehlgeschlagen", "");
        events_->warn("measurement.scan_failed", "WLAN-Scan fehlgeschlagen", "wifi");
    }
}

bool RfScanner::requestScan() {
    if (scanning_) return false;
    if (WiFi.scanComplete() == WIFI_SCAN_RUNNING) return false;
    attemptCount_++;
    int16_t rc = WiFi.scanNetworks(true, true);
    scanning_ = (rc == WIFI_SCAN_RUNNING || rc >= 0);
    if (scanning_) lastScanStart_ = millis();
    if (rc == WIFI_SCAN_FAILED) {
        noteFail();
        return false;
    }
    if (rc >= 0) {
        processScan(rc);
        WiFi.scanDelete();
        scanning_ = false;
    }
    return scanning_ || lastScanOk_;
}

bool RfScanner::due() const {
    unsigned long interval = (unsigned long)(cfg_ ? cfg_->wifiScanIntervalS : 20) * 1000UL;
    if (interval < 8000UL) interval = 8000UL;
    return !scanning_ && (millis() - lastScanStart_ >= interval);
}

uint32_t RfScanner::scanAgeS() const {
    if (lastScanTs_ == 0) return 0xFFFFFFFFu;
    uint32_t now = millis() / 1000UL;
    return now >= lastScanTs_ ? now - lastScanTs_ : 0;
}

float RfScanner::failRatePct() const {
    if (attemptCount_ == 0) return 0;
    return 100.0f * (float)failCount_ / (float)attemptCount_;
}

void RfScanner::loop(bool mayStart) {
    int16_t n = WiFi.scanComplete();
    if (n == WIFI_SCAN_RUNNING) {
        scanning_ = true;
        if (millis() - lastScanStart_ >= 15000UL) {
            Serial.println("[WIFI] scan timeout — force delete");
            WiFi.scanDelete();
            scanning_ = false;
            noteFail();
        }
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
        noteFail();
    }
    if (mayStart && due()) requestScan();
}

int RfScanner::findIndex(const char* bssid) const {
    if (!aps_ || !bssid) return -1;
    for (uint8_t i = 0; i < tracked_; i++) {
        if (strcasecmp(aps_[i].bssid, bssid) == 0) return (int)i;
    }
    return -1;
}

int RfScanner::allocSlot() {
    if (!aps_) return -1;
    if (tracked_ < capacity_) return (int)tracked_++;
    // Evict oldest disappeared first, else oldest lastSeen.
    int best = 0;
    for (uint8_t i = 1; i < tracked_; i++) {
        bool iGone = aps_[i].life == ApLife::Disappeared;
        bool bGone = aps_[best].life == ApLife::Disappeared;
        if (iGone && !bGone) best = i;
        else if (iGone == bGone && aps_[i].lastSeen < aps_[best].lastSeen) best = i;
    }
    return best;
}

void RfScanner::emitDiff(uint8_t idx, bool disappeared) {
    if (!events_ || !aps_ || idx >= tracked_) return;
    const ApRecord& ap = aps_[idx];
    if (disappeared) {
        events_->warn("wifi.ap_lost", "WLAN verschwunden", ap.bssid);
    } else {
        String d = String(ap.ssid) + " " + ap.bssid;
        events_->info("wifi.new_ap", "Neues WLAN erkannt", d.c_str());
    }
}

void RfScanner::recomputeLife() {
    lifeNew_ = lifePersistent_ = lifeFlapping_ = lifeDisappeared_ = 0;
    apCount_ = 0;
    knownCount_ = 0;
    strongest_ = -127;
    float sum = 0;
    uint32_t now = millis() / 1000UL;
    if (!aps_) return;
    for (uint8_t i = 0; i < tracked_; i++) {
        ApRecord& ap = aps_[i];
        uint32_t age = now >= ap.firstSeen ? now - ap.firstSeen : 0;
        if (ap.missStreak >= 1) {
            ap.life = ApLife::Disappeared;
            lifeDisappeared_++;
        } else if (ap.flaps >= 3) {
            ap.life = ApLife::Flapping;
            lifeFlapping_++;
            apCount_++;
        } else if (ap.seenCount <= 2 && age < 180) {
            ap.life = ApLife::New;
            lifeNew_++;
            apCount_++;
        } else {
            ap.life = ApLife::Persistent;
            lifePersistent_++;
            apCount_++;
        }
        if (ap.life != ApLife::Disappeared) {
            if (ap.known) knownCount_++;
            if (ap.rssi > strongest_) strongest_ = ap.rssi;
            sum += ap.rssi;
        }
    }
    avgRssi_ = apCount_ ? (sum / apCount_) : 0;
}

void RfScanner::prune(uint32_t nowS) {
    if (!aps_) return;
    uint8_t w = 0;
    for (uint8_t i = 0; i < tracked_; i++) {
        uint32_t age = nowS >= aps_[i].lastSeen ? nowS - aps_[i].lastSeen : 0;
        // Keep disappeared briefly for UI; drop after 3 minutes unseen.
        if (age > 180) continue;
        if (w != i) aps_[w] = aps_[i];
        w++;
    }
    tracked_ = w;
}

void RfScanner::processScan(int16_t n) {
    if (!aps_) return;
    uint32_t dur = millis() - lastScanStart_;
    lastDurationMs_ = dur;
    totalBusyMs_ += dur;
    successCount_++;
    lastScanOk_ = true;
    uint32_t now = millis() / 1000UL;

    bool hit[96];
    memset(hit, 0, sizeof(hit));

    int16_t limit = n > (int16_t)capacity_ ? (int16_t)capacity_ : n;
    for (int16_t i = 0; i < limit; i++) {
        String ssid = WiFi.SSID(i);
        String bssid = WiFi.BSSIDstr(i);
        int idx = findIndex(bssid.c_str());
        bool isNew = false;
        if (idx < 0) {
            idx = allocSlot();
            if (idx < 0) continue;
            ApRecord& ap = aps_[idx];
            memset(&ap, 0, sizeof(ap));
            strncpy(ap.bssid, bssid.c_str(), sizeof(ap.bssid) - 1);
            ap.firstSeen = now;
            ap.seenCount = 0;
            ap.missStreak = 0;
            ap.flaps = 0;
            isNew = true;
        } else if (aps_[idx].missStreak > 0) {
            if (aps_[idx].flaps < 250) aps_[idx].flaps++;
        }
        ApRecord& ap = aps_[idx];
        strncpy(ap.ssid, ssid.c_str(), sizeof(ap.ssid) - 1);
        ap.ssid[sizeof(ap.ssid) - 1] = 0;
        ap.rssi = (int8_t)WiFi.RSSI(i);
        ap.channel = (uint8_t)WiFi.channel(i);
        ap.enc = (uint8_t)WiFi.encryptionType(i);
        ap.hidden = ssid.length() == 0;
        ap.known = isKnownSsid(ssid);
        ap.lastSeen = now;
        ap.missStreak = 0;
        if (ap.seenCount < 65535) ap.seenCount++;
        if (idx >= 0 && idx < 96) hit[idx] = true;
        if (isNew && successCount_ > 1) emitDiff((uint8_t)idx, false);
    }

    for (uint8_t i = 0; i < tracked_; i++) {
        if (i < 96 && hit[i]) continue;
        if (aps_[i].missStreak < 250) aps_[i].missStreak++;
        if (aps_[i].missStreak == 1) emitDiff(i, true);
    }

    prune(now);
    recomputeLife();
    lastScanTs_ = now;
    if (history_) history_->push(HistoryStore::AP_COUNT, (float)apCount_);
}

void RfScanner::channelOccupancy(uint8_t out[14]) const {
    memset(out, 0, 14);
    if (!aps_) return;
    for (uint8_t i = 0; i < tracked_; i++) {
        if (aps_[i].life == ApLife::Disappeared) continue;
        uint8_t ch = aps_[i].channel;
        if (ch >= 1 && ch <= 14) out[ch - 1]++;
    }
}

void RfScanner::channelStats(uint8_t ch, uint8_t& count, int8_t& maxRssi, int8_t& avgRssi,
                             uint8_t& known, uint8_t& unknown) const {
    count = 0;
    known = 0;
    unknown = 0;
    maxRssi = -127;
    int32_t sum = 0;
    if (!aps_) { avgRssi = 0; return; }
    for (uint8_t i = 0; i < tracked_; i++) {
        if (aps_[i].life == ApLife::Disappeared) continue;
        if (aps_[i].channel != ch) continue;
        count++;
        if (aps_[i].rssi > maxRssi) maxRssi = aps_[i].rssi;
        sum += aps_[i].rssi;
        if (aps_[i].known) known++;
        else unknown++;
    }
    avgRssi = count ? (int8_t)(sum / count) : 0;
}

void RfScanner::resultsToJson(JsonArray arr) const {
    if (!aps_) return;
    for (uint8_t i = 0; i < tracked_; i++) {
        JsonObject o = arr.add<JsonObject>();
        o["ssid"] = aps_[i].ssid;
        o["bssid"] = aps_[i].bssid;
        o["rssi"] = aps_[i].rssi;
        o["channel"] = aps_[i].channel;
        o["enc"] = aps_[i].enc;
        o["hidden"] = aps_[i].hidden;
        o["known"] = aps_[i].known;
        o["firstSeen"] = aps_[i].firstSeen;
        o["lastSeen"] = aps_[i].lastSeen;
        o["seenCount"] = aps_[i].seenCount;
        o["missStreak"] = aps_[i].missStreak;
        o["flaps"] = aps_[i].flaps;
        o["life"] = lifeName(aps_[i].life);
    }
}

void RfScanner::appendLifeJson(JsonObject target) const {
    target["tracked"] = tracked_;
    target["visible"] = apCount_;
    target["new"] = lifeNew_;
    target["persistent"] = lifePersistent_;
    target["flapping"] = lifeFlapping_;
    target["disappeared"] = lifeDisappeared_;
}

void RfScanner::channelsToJson(JsonArray arr) const {
    for (uint8_t i = 0; i < 14; i++) {
        uint8_t count = 0, known = 0, unknown = 0;
        int8_t maxRssi = -127, avgRssi = 0;
        channelStats(i + 1, count, maxRssi, avgRssi, known, unknown);
        uint8_t nNew = 0, nPers = 0, nFlap = 0, nGone = 0;
        if (aps_) {
            for (uint8_t a = 0; a < tracked_; a++) {
                if (aps_[a].channel != (i + 1)) continue;
                switch (aps_[a].life) {
                    case ApLife::New: nNew++; break;
                    case ApLife::Persistent: nPers++; break;
                    case ApLife::Flapping: nFlap++; break;
                    case ApLife::Disappeared: nGone++; break;
                }
            }
        }
        JsonObject o = arr.add<JsonObject>();
        o["ch"] = i + 1;
        o["count"] = count;
        o["maxRssi"] = count ? maxRssi : 0;
        o["avgRssi"] = count ? avgRssi : 0;
        o["known"] = known;
        o["unknown"] = unknown;
        o["new"] = nNew;
        o["persistent"] = nPers;
        o["flapping"] = nFlap;
        o["disappeared"] = nGone;
        const char* pressure = "none";
        if (count >= 8) pressure = "high";
        else if (count >= 4) pressure = "medium";
        else if (count > 0) pressure = "low";
        o["pressure"] = pressure;
    }
}
