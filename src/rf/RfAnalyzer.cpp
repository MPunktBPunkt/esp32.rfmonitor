#include "RfAnalyzer.h"
#include "core/HistoryStore.h"
#include "core/HwCaps.h"
#include "core/TimeBase.h"
#include "BuildFlags.h"
#include <math.h>
#include <string.h>
#include <ArduinoJson.h>

void RfAnalyzer::begin(ConfigStore* cfg, EventLog* events, HistoryStore* history, RfScanner* scanner, BleTracker* ble) {
    cfg_ = cfg;
    events_ = events;
    history_ = history;
    scanner_ = scanner;
    ble_ = ble;
    location_.begin(cfg);
    session_.begin();
    lastWifi_ = WiFi.status();
    lastRssi_ = (int8_t)WiFi.RSSI();
}

void RfAnalyzer::loop() {
    wl_status_t st = WiFi.status();
    if (lastWifi_ == WL_CONNECTED && st != WL_CONNECTED) {
        disconnects_++;
        if (stability_ > 8) stability_ -= 8;
        if (events_) events_->warn("system.reconnect", "WLAN-Verbindung verloren", "");
        if (events_) events_->warn("wifi.own_ap_lost", "Eigene WLAN-Verbindung kritisch", "");
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

void RfAnalyzer::putProxy(JsonObject parent, const char* key, uint8_t value, const char* basis) {
    JsonObject o = parent[key].to<JsonObject>();
    o["value"] = value;
    o["kind"] = "proxy";
    o["basis"] = basis;
}

void RfAnalyzer::putParts(JsonObject parts, const RfScoreBreakdown& b) {
    parts["ownLink"] = b.ownLink;
    parts["rssi"] = b.ownLink;
    parts["jitter"] = b.jitter;
    parts["density"] = b.density;
    parts["pressure"] = b.pressure;
    parts["congestion"] = b.pressure;
    parts["occupancy"] = b.occupancy;
    parts["stability"] = b.stability;
    parts["disconnect"] = b.disconnect;
    parts["qualityGate"] = b.qualityGate;
}

void RfAnalyzer::recomputeQuality() {
    int q = 100;
    uint32_t wifiInterval = cfg_ ? (uint32_t)cfg_->wifiScanIntervalS : 20;
    if (wifiInterval < 8) wifiInterval = 8;

    if (!scanner_ || scanner_->successCount() == 0) {
        q -= 40;
    } else {
        uint32_t age = scanner_->scanAgeS();
        if (age != 0xFFFFFFFFu && age > wifiInterval * 2) q -= 20;
        q -= (int)(scanner_->failRatePct() * 0.4f);
    }

    if (cfg_ && cfg_->bleEnabled && ble_ && ble_->ready()) {
        q -= (int)(ble_->failRatePct() * 0.3f);
    }

    if (rssiCount_ < 6) q -= 15;
    // Prefer recent duty window so boot-spike lifetime doesn't permanently tank quality.
    float dutyRecent = radioDuty5mPct_ > 0.0f ? radioDuty5mPct_ : radioDutyPct_;
    if (dutyRecent > 40.0f) q -= 10;
    if (q < 0) q = 0;
    if (q > 100) q = 100;
    quality_ = (uint8_t)q;
}

void RfAnalyzer::updateRadioDuty() {
    uint32_t busy = 0;
    if (scanner_) busy += scanner_->totalBusyMs();
    if (ble_) busy += ble_->totalBusyMs();

    uint32_t upMs = millis();
    if (upMs < 1000UL) upMs = 1000UL;
    radioDutyPct_ = 100.0f * (float)busy / (float)upMs;
    if (radioDutyPct_ > 100.0f) radioDutyPct_ = 100.0f;

    uint32_t delta = 0;
    if (busy >= lastBusyMs_) delta = busy - lastBusyMs_;
    lastBusyMs_ = busy;
    busyThisMin_ += delta;

    if (minuteMarkMs_ == 0) minuteMarkMs_ = millis();
    if (millis() - minuteMarkMs_ >= 60000UL) {
        busyPerMin_[busyMinHead_] = busyThisMin_;
        busyMinHead_ = (uint8_t)((busyMinHead_ + 1) % 60);
        if (busyMinFilled_ < 60) busyMinFilled_++;
        busyThisMin_ = 0;
        minuteMarkMs_ = millis();
    }

    auto windowPct = [&](uint8_t minutes) -> float {
        if (minutes == 0) return 0;
        uint8_t n = busyMinFilled_ < minutes ? busyMinFilled_ : minutes;
        if (n == 0) {
            // Warm-up: use in-progress minute + lifetime as fallback.
            return radioDutyPct_;
        }
        uint32_t sum = busyThisMin_;
        for (uint8_t i = 0; i < n; i++) {
            uint8_t idx = (uint8_t)((busyMinHead_ + 60 - 1 - i) % 60);
            sum += busyPerMin_[idx];
        }
        uint32_t windowMs = (uint32_t)(n + 1) * 60000UL;
        if (windowMs < 1000UL) windowMs = 1000UL;
        float pct = 100.0f * (float)sum / (float)windowMs;
        if (pct > 100.0f) pct = 100.0f;
        return pct;
    };
    radioDuty5mPct_ = windowPct(5);
    radioDuty1hPct_ = windowPct(60);
}

const char* RfAnalyzer::condName(CondState s) {
    switch (s) {
        case CondState::Pending: return "pending";
        case CondState::Active: return "active";
        case CondState::Acked: return "acked";
        case CondState::Recovering: return "recovering";
        default: return "inactive";
    }
}

bool RfAnalyzer::tickHyst(Hyst& h, bool enterCond, bool exitCond,
                          unsigned long enterMs, unsigned long exitMs, unsigned long cooldownMs,
                          const char* code, const char* enterType, const char* enterTitle,
                          const char* exitType, const char* exitTitle, const char* detail) {
    unsigned long now = millis();
    h.code = code ? code : "";
    if (!h.active) {
        if (enterCond) {
            if (h.enterSince == 0) {
                h.enterSince = now;
                h.state = CondState::Pending;
            }
            if (now - h.enterSince >= enterMs && (h.lastEmit == 0 || now - h.lastEmit >= cooldownMs)) {
                h.active = true;
                h.acked = false;
                h.clearSince = 0;
                h.lastEmit = now;
                h.activeSince = now;
                h.state = CondState::Active;
                if (events_) events_->warn(enterType, enterTitle, detail);
                return true;
            }
        } else {
            h.enterSince = 0;
            h.state = CondState::Inactive;
        }
    } else {
        if (exitCond) {
            if (h.clearSince == 0) {
                h.clearSince = now;
                h.state = CondState::Recovering;
            }
            if (now - h.clearSince >= exitMs) {
                h.active = false;
                h.acked = false;
                h.enterSince = 0;
                h.clearSince = 0;
                h.lastEmit = now;
                h.state = CondState::Inactive;
                if (events_) events_->info(exitType, exitTitle, detail);
                return true;
            }
        } else {
            h.clearSince = 0;
            h.state = h.acked ? CondState::Acked : CondState::Active;
        }
    }
    return false;
}

RfAnalyzer::Hyst* RfAnalyzer::findAlert(const char* code) {
    if (!code) return nullptr;
    if (!strcmp(code, "score_low")) return &hystScore_;
    if (!strcmp(code, "channel_pressure_high")) return &hystPressure_;
    if (!strcmp(code, "data_quality_low")) return &hystQuality_;
    if (!strcmp(code, "radio_budget_high")) return &hystDuty_;
    if (!strcmp(code, "baseline_deviation")) return &hystDeviation_;
    return nullptr;
}

bool RfAnalyzer::ackAlert(const char* code) {
    Hyst* h = findAlert(code);
    if (!h || !h->active) return false;
    h->acked = true;
    h->state = CondState::Acked;
    return true;
}

bool RfAnalyzer::ackAllAlerts() {
    bool any = false;
    any |= ackAlert("score_low");
    any |= ackAlert("channel_pressure_high");
    any |= ackAlert("data_quality_low");
    any |= ackAlert("radio_budget_high");
    any |= ackAlert("baseline_deviation");
    return any;
}

void RfAnalyzer::putAlertCond(JsonObject parent, const char* key, const Hyst& h) const {
    JsonObject o = parent[key].to<JsonObject>();
    o["code"] = h.code;
    o["state"] = condName(h.state);
    o["active"] = h.active;
    o["acked"] = h.acked;
    o["activeForMs"] = h.active && h.activeSince ? (millis() - h.activeSince) : 0;
}

void RfAnalyzer::updateAlerts() {
    uint8_t warnScore = cfg_ ? cfg_->rfScoreWarn : 40;
    uint8_t exitScore = warnScore + 8;
    if (exitScore > 100) exitScore = 100;
    char detail[16];

    snprintf(detail, sizeof(detail), "%u", (unsigned)score_);
    tickHyst(hystScore_, score_ < warnScore, score_ > exitScore,
             30000UL, 60000UL, 120000UL, "score_low",
             "rf.score_low", "RF-Score schlecht",
             "rf.score_recover", "RF-Score erholt", detail);

    uint8_t congWarn = cfg_ ? cfg_->congestionWarn : 70;
    uint8_t congExit = congWarn > 10 ? (uint8_t)(congWarn - 10) : 0;
    snprintf(detail, sizeof(detail), "%u", (unsigned)congestion_);
    tickHyst(hystPressure_, congestion_ >= congWarn, congestion_ < congExit,
             30000UL, 60000UL, 120000UL, "channel_pressure_high",
             "rf.channel_pressure_high", "Kanaldruck-Proxy hoch",
             "rf.channel_pressure_recover", "Kanaldruck-Proxy normal", detail);

    snprintf(detail, sizeof(detail), "%u", (unsigned)quality_);
    tickHyst(hystQuality_, quality_ < 50, quality_ > 60,
             30000UL, 60000UL, 120000UL, "data_quality_low",
             "measurement.data_quality_low", "Messqualitaet niedrig",
             "measurement.data_quality_recover", "Messqualitaet ok", detail);

    // Alerts use 5m window (not lifetime), so boot scan bursts don't stick.
    float dutyAlert = radioDuty5mPct_ > 0.0f ? radioDuty5mPct_ : radioDutyPct_;
    snprintf(detail, sizeof(detail), "%.0f", dutyAlert);
    tickHyst(hystDuty_, dutyAlert > 45.0f, dutyAlert < 35.0f,
             45000UL, 90000UL, 180000UL, "radio_budget_high",
             "measurement.radio_budget_high", "Radio-Zeitbudget hoch",
             "measurement.radio_budget_ok", "Radio-Zeitbudget ok", detail);

    if (location_.valid()) {
        snprintf(detail, sizeof(detail), "%d", (int)location_.deviation());
        tickHyst(hystDeviation_, location_.deviation() <= -25, location_.deviation() >= -10,
                 45000UL, 90000UL, 180000UL, "baseline_deviation",
                 "rf.baseline_deviation", "Abweichung vom Standortnormal",
                 "rf.baseline_recover", "Lage nahe Standortnormal", detail);
    }
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

    updateRadioDuty();
    recomputeQuality();

    RfScoreFeatures feat;
    feat.ownRssi = rssi;
    feat.rssiJitter = rssiJitter_;
    feat.densityProxy = density_;
    feat.pressureProxy = congestion_;
    feat.occupancyProxy = channelLoad_;
    feat.stability = stability_;
    feat.connected = WiFi.status() == WL_CONNECTED;
    feat.measurementQuality = quality_;

    RfScoreResult v1 = RfScore::computeV1(feat);
    RfScoreResult v2 = RfScore::computeV2(feat);
    scoreV1_ = v1.score;
    scoreV2_ = v2.score;
    partsV1_ = v1.parts;
    partsV2_ = v2.parts;

    uint8_t prev = score_;
#if RFMON_SCORE_VERSION >= 2
    score_ = scoreV2_;
    penRssi_ = partsV2_.ownLink;
    penJitter_ = partsV2_.jitter;
    penCongestion_ = partsV2_.pressure;
    penDensity_ = partsV2_.density;
    penStability_ = partsV2_.stability;
    penDisconnect_ = partsV2_.disconnect;
#else
    score_ = scoreV1_;
    penRssi_ = partsV1_.ownLink;
    penJitter_ = partsV1_.jitter;
    penCongestion_ = partsV1_.pressure;
    penDensity_ = partsV1_.density;
    penStability_ = partsV1_.stability;
    penDisconnect_ = partsV1_.disconnect;
#endif

    uint8_t bleN = ble_ ? ble_->deviceCount() : 0;
    location_.loop(score_, density_, congestion_, channelLoad_, aps, bleN, (float)rssi, ch);
    updateAlerts();

    if (history_) {
        history_->push(HistoryStore::RSSI, (float)rssi);
        history_->push(HistoryStore::RF_SCORE, (float)score_);
        history_->push(HistoryStore::DATA_QUALITY, (float)quality_);
        history_->pushChannels(ch);
    }

    session_.noteSample(score_, quality_, location_.deviation(), aps, bleN, congestion_,
                        channelLoad_, density_, rssiJitter_, stability_,
                        WiFi.status() == WL_CONNECTED,
                        (float)rssi, radioDuty5mPct_ > 0 ? radioDuty5mPct_ : radioDutyPct_, 2);

    int drop = cfg_ ? cfg_->rssiWarnDrop : 12;
    if (lastRssi_ != 0 && rssi <= lastRssi_ - drop && events_) {
        events_->warn("wifi.rssi_drop", "RSSI deutlich gefallen", String(rssi).c_str());
    }
    if (lastRssi_ != 0 && lastRssi_ <= -70 && rssi >= lastRssi_ + drop && events_) {
        events_->info("wifi.rssi_recover", "RSSI erholt", String(rssi).c_str());
    }
    lastRssi_ = rssi;

    if (prev >= 60 && score_ + 15 < prev && events_) {
        events_->warn("rf.score_drop", "RF-Score stark gefallen", String(score_).c_str());
    }
}

bool RfAnalyzer::anyAlert() const {
    auto open = [](const Hyst& h) { return h.active && !h.acked; };
    return open(hystScore_) || open(hystPressure_) || open(hystQuality_)
           || open(hystDuty_) || open(hystDeviation_)
           || WiFi.status() != WL_CONNECTED;
}

void RfAnalyzer::appendAlertState(JsonObject target) const {
    target["contract"] = "rfmonitor-hub-v2";
    target["any"] = anyAlert();
    JsonObject flags = target["flags"].to<JsonObject>();
    flags["score_low"] = hystScore_.active;
    flags["channel_pressure_high"] = hystPressure_.active;
    flags["data_quality_low"] = hystQuality_.active;
    flags["radio_budget_high"] = hystDuty_.active;
    flags["baseline_deviation"] = hystDeviation_.active;
    flags["wifi_disconnected"] = WiFi.status() != WL_CONNECTED;
    JsonObject cond = target["conditions"].to<JsonObject>();
    putAlertCond(cond, "score_low", hystScore_);
    putAlertCond(cond, "channel_pressure_high", hystPressure_);
    putAlertCond(cond, "data_quality_low", hystQuality_);
    putAlertCond(cond, "radio_budget_high", hystDuty_);
    putAlertCond(cond, "baseline_deviation", hystDeviation_);
}

void RfAnalyzer::appendHubContract(JsonObject target) const {
    target["contract"] = "rfmonitor-hub-v2";
    target["fwType"] = "rfmonitor";
    target["score"] = score_;
    target["scoreVersion"] = RFMON_SCORE_VERSION;
    target["scoreV1"] = scoreV1_;
    target["scoreV2"] = scoreV2_;
    target["measurementQuality"] = quality_;
    target["locationDeviation"] = location_.deviation();
    target["channelPressure"] = congestion_;
    target["apCount"] = scanner_ ? scanner_->apCount() : 0;
    target["bleCount"] = ble_ ? ble_->deviceCount() : 0;
    target["radioDutyPct"] = radioDutyPct_;
    target["radioDuty5mPct"] = radioDuty5mPct_;
    target["radioDuty1hPct"] = radioDuty1hPct_;
    target["wifiRssi"] = WiFi.RSSI();
    target["locationValid"] = location_.valid();
    appendAlertState(target["alert"].to<JsonObject>());
}

void RfAnalyzer::appendWhyJson(JsonObject target) const {
    const RfScoreBreakdown& parts = RFMON_SCORE_VERSION >= 2 ? partsV2_ : partsV1_;
    target["score"] = score_;
    target["scoreVersion"] = RFMON_SCORE_VERSION;
    target["measurementQuality"] = quality_;
    target["locationDeviation"] = location_.deviation();

    struct Row { const char* key; uint8_t value; bool proxy; };
    Row rows[] = {
        {"ownLink", parts.ownLink, false},
        {"jitter", parts.jitter, false},
        {"density", parts.density, true},
        {"pressure", parts.pressure, true},
        {"occupancy", parts.occupancy, true},
        {"stability", parts.stability, false},
        {"disconnect", parts.disconnect, false},
        {"qualityGate", parts.qualityGate, false},
    };
    const char* strongest = "none";
    uint8_t strongestV = 0;
    JsonArray arr = target["penalties"].to<JsonArray>();
    for (auto& r : rows) {
        JsonObject o = arr.add<JsonObject>();
        o["key"] = r.key;
        o["value"] = r.value;
        o["proxy"] = r.proxy;
        if (r.value > strongestV) {
            strongestV = r.value;
            strongest = r.key;
        }
    }
    target["strongestPenalty"] = strongest;
    target["strongestPenaltyValue"] = strongestV;
}

void RfAnalyzer::appendFindingsJson(JsonArray arr) const {
    auto add = [&](const char* level, const char* code, const char* text) {
        JsonObject o = arr.add<JsonObject>();
        o["level"] = level;
        o["code"] = code;
        o["text"] = text;
    };
    if (WiFi.status() == WL_CONNECTED) add("ok", "wifi.own_ok", "Eigenes WLAN verbunden");
    else add("bad", "wifi.own_lost", "Eigenes WLAN getrennt");

    if (rssiMin_ <= -75) add("warn", "wifi.rssi_weak", "Eigene Verbindung schwach");
    else add("ok", "wifi.rssi_ok", "Eigenes RSSI im Rahmen");

    if (congestion_ >= 70) add("warn", "rf.pressure_high", "Kanaldruck-Proxy hoch");
    else add("ok", "rf.pressure_ok", "Kanaldruck-Proxy normal");

    if (quality_ < 50) add("warn", "meas.quality_low", "Messqualitaet niedrig");
    else add("ok", "meas.quality_ok", "Messqualitaet ok");

    if (location_.valid()) {
        if (location_.stale()) add("info", "loc.stale", "Standort-Baseline aelter als 90 Tage");
        if (location_.deviation() <= -25) add("warn", "loc.dev_high", "Lage weicht vom Standortnormal ab");
        else if (location_.driftMagnitude() >= 20.0f) add("info", "loc.drift", "Baseline-Drift erhoeht");
        else add("ok", "loc.dev_ok", "Lage nahe Standortnormal");
    } else if (location_.capturing()) {
        add("info", "loc.capturing", "Standort-Baseline wird aufgenommen");
    } else {
        add("info", "loc.missing", "Keine Standort-Baseline");
    }

    if (scanner_ && scanner_->unknownCount() > 0) {
        char buf[48];
        snprintf(buf, sizeof(buf), "%u unbekannte APs", (unsigned)scanner_->unknownCount());
        add("info", "wifi.unknown_aps", buf);
    }
    if (scanner_ && (scanner_->newCount() || scanner_->flappingCount() || scanner_->disappearedCount())) {
        char buf[72];
        snprintf(buf, sizeof(buf), "APs new %u · flap %u · gone %u",
                 (unsigned)scanner_->newCount(), (unsigned)scanner_->flappingCount(),
                 (unsigned)scanner_->disappearedCount());
        add("info", "wifi.ap_life", buf);
    }
    if (ble_ && ble_->newCount() > 0) {
        char buf[48];
        snprintf(buf, sizeof(buf), "%u neue BLE-Geraete im Fenster", (unsigned)ble_->newCount());
        add("info", "ble.new", buf);
    }
}

void RfAnalyzer::appendObservationJson(JsonObject target) const {
    JsonObject obs = target["observation"].to<JsonObject>();
    JsonObject wifi = obs["wifi"].to<JsonObject>();
    wifi["ownRssi"] = WiFi.RSSI();
    wifi["connected"] = WiFi.status() == WL_CONNECTED;
    if (scanner_) {
        wifi["apCount"] = scanner_->apCount();
        wifi["knownApCount"] = scanner_->knownCount();
        wifi["unknownApCount"] = scanner_->unknownCount();
        wifi["strongestRssi"] = scanner_->strongestRssi();
        wifi["scanDurationMs"] = scanner_->lastScanDurationMs();
        wifi["scanAgeS"] = scanner_->scanAgeS() == 0xFFFFFFFFu ? -1 : (int)scanner_->scanAgeS();
        wifi["scanOk"] = scanner_->lastScanOk();
        wifi["scanAttempts"] = scanner_->attemptCount();
        wifi["scanFailures"] = scanner_->failCount();
        wifi["scanSuccesses"] = scanner_->successCount();
        wifi["failRatePct"] = scanner_->failRatePct();
        wifi["capacity"] = (uint32_t)scanner_->capacity();
        scanner_->appendLifeJson(wifi["apLife"].to<JsonObject>());
    }
    JsonObject bleo = obs["ble"].to<JsonObject>();
    if (ble_) {
        bleo["deviceCount"] = ble_->deviceCount();
        bleo["beaconCount"] = ble_->beaconCount();
        bleo["newCount"] = ble_->newCount();
        bleo["scanDurationMs"] = ble_->lastScanDurationMs();
        bleo["scanAgeS"] = ble_->scanAgeS() == 0xFFFFFFFFu ? -1 : (int)ble_->scanAgeS();
        bleo["scanAttempts"] = ble_->attemptCount();
        bleo["scanFailures"] = ble_->failCount();
        bleo["scanSuccesses"] = ble_->successCount();
        bleo["failRatePct"] = ble_->failRatePct();
        bleo["ready"] = ble_->ready();
        bleo["capacity"] = (uint32_t)ble_->capacity();
    }
    JsonObject radio = obs["radio"].to<JsonObject>();
    radio["wifiBusyMs"] = scanner_ ? scanner_->totalBusyMs() : 0;
    radio["bleBusyMs"] = ble_ ? ble_->totalBusyMs() : 0;
    radio["dutyCyclePct"] = radioDutyPct_;           // lifetime (compat)
    radio["dutyLifetimePct"] = radioDutyPct_;
    radio["duty5mPct"] = radioDuty5mPct_;
    radio["duty1hPct"] = radioDuty1hPct_;
    radio["wifiScanning"] = scanner_ ? scanner_->busy() : false;
    radio["bleScanning"] = ble_ ? ble_->busy() : false;
    JsonObject link = obs["link"].to<JsonObject>();
    link["rssiSampleCount"] = rssiCount_;
    link["rssiAvg"] = rssiAvg_;
    link["rssiJitter"] = rssiJitter_;
    link["rssiMin"] = rssiMin_;
    link["rssiMax"] = rssiMax_;
    link["disconnects"] = disconnects_;
    link["stability"] = stability_;
    link["connected"] = WiFi.status() == WL_CONNECTED;

    JsonObject derived = target["derived"].to<JsonObject>();
    putProxy(derived, "density", density_, "ap_count");
    putProxy(derived, "channelPressure", congestion_, "max_aps_per_channel");
    putProxy(derived, "channelOccupancy", channelLoad_, "occupied_2g_channels");

    JsonObject assess = target["assessment"].to<JsonObject>();
    assess["score"] = score_;
    assess["scoreVersion"] = RFMON_SCORE_VERSION;
    assess["scoreV1"] = scoreV1_;
    assess["scoreV2"] = scoreV2_;
    assess["measurementQuality"] = quality_;
    assess["locationDeviation"] = location_.deviation();
    putParts(assess["scoreParts"].to<JsonObject>(), RFMON_SCORE_VERSION >= 2 ? partsV2_ : partsV1_);
    putParts(assess["scorePartsV1"].to<JsonObject>(), partsV1_);
    putParts(assess["scorePartsV2"].to<JsonObject>(), partsV2_);
    appendWhyJson(assess["why"].to<JsonObject>());

    location_.appendJson(target["location"].to<JsonObject>());
    session_.appendStatusJson(target["session"].to<JsonObject>());
    appendFindingsJson(target["findings"].to<JsonArray>());
    appendAlertState(target["alert"].to<JsonObject>());
}

void RfAnalyzer::appendSnapshotJson(JsonObject target) const {
    target["kind"] = "coexistence_snapshot";
    target["fwVersion"] = FW_VERSION;
    target["scoreVersion"] = RFMON_SCORE_VERSION;
    target["board"] = RFMON_BOARD_ID;
    target["location"] = cfg_ ? cfg_->locationName : "";
    target["timestampS"] = TimeBase::uptimeS();  // legacy = uptime
    TimeBase::appendJson(target["time"].to<JsonObject>());
    target["rfScore"] = score_;
    target["scoreV1"] = scoreV1_;
    target["scoreV2"] = scoreV2_;
    target["measurementQuality"] = quality_;
    target["locationDeviation"] = location_.deviation();
    target["channelPressure"] = congestion_;
    target["channelOccupancy"] = channelLoad_;
    target["density"] = density_;
    target["radioDutyPct"] = radioDutyPct_;
    target["radioDuty5mPct"] = radioDuty5mPct_;
    target["radioDuty1hPct"] = radioDuty1hPct_;
    appendWhyJson(target["why"].to<JsonObject>());
    JsonObject wifi = target["wifi"].to<JsonObject>();
    wifi["apCount"] = scanner_ ? scanner_->apCount() : 0;
    wifi["knownApCount"] = scanner_ ? scanner_->knownCount() : 0;
    wifi["ownRssi"] = WiFi.RSSI();
    wifi["ssid"] = WiFi.SSID();
    wifi["channel"] = WiFi.channel();
    if (scanner_) scanner_->channelsToJson(wifi["channels"].to<JsonArray>());
    JsonObject bleo = target["ble"].to<JsonObject>();
    bleo["deviceCount"] = ble_ ? ble_->deviceCount() : 0;
    bleo["beaconCount"] = ble_ ? ble_->beaconCount() : 0;
    HwCaps::appendJson(target["hw"].to<JsonObject>());
    location_.appendJson(target["locationProfile"].to<JsonObject>());
}

void RfAnalyzer::appendStatusJson(JsonObject target) const {
    target["rfScore"] = score_;
    target["scoreVersion"] = RFMON_SCORE_VERSION;
    target["scoreV1"] = scoreV1_;
    target["scoreV2"] = scoreV2_;
    target["measurementQuality"] = quality_;
    target["locationDeviation"] = location_.deviation();
    target["wifiRssi"] = WiFi.RSSI();
    target["wifiRssiAvg"] = rssiAvg_;
    target["wifiRssiMin"] = rssiMin_;
    target["wifiRssiMax"] = rssiMax_;
    target["wifiRssiJitter"] = rssiJitter_;
    target["channelLoad"] = channelLoad_;
    target["channelCongestion"] = congestion_;
    target["channelPressureProxy"] = congestion_;
    target["channelOccupancyProxy"] = channelLoad_;
    target["apDensity"] = density_;
    target["stability"] = stability_;
    target["disconnects"] = disconnects_;
    target["radioDutyPct"] = radioDutyPct_;
    target["radioDuty5mPct"] = radioDuty5mPct_;
    target["radioDuty1hPct"] = radioDuty1hPct_;
    putParts(target["scoreParts"].to<JsonObject>(), RFMON_SCORE_VERSION >= 2 ? partsV2_ : partsV1_);
    appendWhyJson(target["why"].to<JsonObject>());
    if (scanner_) {
        target["apCount"] = scanner_->apCount();
        target["knownApCount"] = scanner_->knownCount();
        target["unknownApCount"] = scanner_->unknownCount();
        target["strongestRssi"] = scanner_->strongestRssi();
        target["scanFailCount"] = scanner_->failCount();
        target["wifiScanDurationMs"] = scanner_->lastScanDurationMs();
        target["wifiScanFailRate"] = scanner_->failRatePct();
        scanner_->appendLifeJson(target["apLife"].to<JsonObject>());
    }
    if (ble_) {
        target["bleCount"] = ble_->deviceCount();
        target["beaconCount"] = ble_->beaconCount();
        target["newBleCount"] = ble_->newCount();
        target["bleScanDurationMs"] = ble_->lastScanDurationMs();
        target["bleScanFailRate"] = ble_->failRatePct();
    }
    appendObservationJson(target);
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
    add("rf_score_v1", (float)scoreV1_, "");
    add("rf_score_v2", (float)scoreV2_, "");
    add("rf_score_version", (float)RFMON_SCORE_VERSION, "");
    add("measurement_quality", (float)quality_, "");
    add("location_deviation", (float)location_.deviation(), "");
    add("ap_count", scanner_ ? (float)scanner_->apCount() : 0, "");
    add("ble_count", ble_ ? (float)ble_->deviceCount() : 0, "");
    add("beacon_count", ble_ ? (float)ble_->beaconCount() : 0, "");
    add("channel_load", (float)channelLoad_, "");
    add("channel_pressure_proxy", (float)congestion_, "");
    add("scan_fail_count", scanner_ ? (float)scanner_->failCount() : 0, "");
    add("radio_duty_pct", radioDutyPct_, "%");
    add("radio_duty_5m_pct", radioDuty5mPct_, "%");
    add("radio_duty_1h_pct", radioDuty1hPct_, "%");
    add("alert_any", anyAlert() ? 1.0f : 0.0f, "");
    add("alert_score_low", hystScore_.active ? 1.0f : 0.0f, "");
    add("alert_pressure_high", hystPressure_.active ? 1.0f : 0.0f, "");
    add("alert_quality_low", hystQuality_.active ? 1.0f : 0.0f, "");
}
