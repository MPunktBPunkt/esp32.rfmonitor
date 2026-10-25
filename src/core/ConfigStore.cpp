#include "ConfigStore.h"
#include <Preferences.h>

static Preferences prefs;

void ConfigStore::applyDefaults() {
    deviceName = DEVICE_NAME_DEFAULT;
    locationName = "Standort";
    hubHost = HUB_HOST_DEFAULT;
    hubPort = HUB_PORT_DEFAULT;
    enableHub = true;
    heartbeatIntervalS = 30;
    logLevel = 2;
    enableMdns = true;
    watchdogS = 300;
    wifiScanIntervalS = 20;
    bleScanIntervalS = 15;
    bleEnabled = true;
    knownSsids = "";
    knownBle = "";
    rssiWarnDrop = 12;
    rfScoreWarn = 40;
    congestionWarn = 70;
    opMode = "normal";
    firstRunDone = false;
}

void ConfigStore::applyModeIntervals() {
    if (opMode == "live") {
        if (wifiScanIntervalS > 10) wifiScanIntervalS = 8;
        if (bleScanIntervalS > 10) bleScanIntervalS = 8;
    } else if (opMode == "diagnostic") {
        wifiScanIntervalS = 10;
        bleScanIntervalS = 8;
    } else {
        if (wifiScanIntervalS < 8) wifiScanIntervalS = 20;
        if (bleScanIntervalS < 6) bleScanIntervalS = 15;
        opMode = "normal";
    }
}

void ConfigStore::begin() {
    applyDefaults();
    load();
    applyModeIntervals();
}

void ConfigStore::load() {
    prefs.begin("esphub", true);
    deviceName = prefs.getString("name", deviceName);
    hubHost = prefs.getString("hub_host", hubHost);
    hubPort = prefs.getInt("hub_port", hubPort);
    bool hadHubName = deviceName.length() > 0;
    prefs.end();

    prefs.begin("rfmon", true);
    uint8_t ver = prefs.getUChar("cfg_ver", 0);
    if (ver == 0) {
        firstRunDone = hadHubName && hubHost.length() > 0;
        prefs.end();
        return;
    }
    firstRunDone = prefs.getBool("wiz_done", true);
    locationName = prefs.getString("loc", locationName);
    enableHub = prefs.getBool("en_hub", enableHub);
    heartbeatIntervalS = prefs.getUShort("hb_s", heartbeatIntervalS);
    logLevel = prefs.getUChar("log_lvl", logLevel);
    enableMdns = prefs.getBool("en_mdns", enableMdns);
    watchdogS = prefs.getUShort("wdt_s", watchdogS);
    wifiScanIntervalS = prefs.getUShort("scan_s", wifiScanIntervalS);
    bleScanIntervalS = prefs.getUShort("ble_s", bleScanIntervalS);
    bleEnabled = prefs.getBool("en_ble", bleEnabled);
    knownSsids = prefs.getString("known_ssid", knownSsids);
    knownBle = prefs.getString("known_ble", knownBle);
    rssiWarnDrop = (int8_t)prefs.getChar("rssi_drop", rssiWarnDrop);
    rfScoreWarn = prefs.getUChar("score_warn", rfScoreWarn);
    congestionWarn = prefs.getUChar("cong_warn", congestionWarn);
    opMode = prefs.getString("mode", opMode);
    prefs.end();
}

void ConfigStore::save() {
    prefs.begin("esphub", false);
    prefs.putString("name", deviceName);
    prefs.putString("hub_host", hubHost);
    prefs.putInt("hub_port", hubPort);
    prefs.end();

    prefs.begin("rfmon", false);
    prefs.putUChar("cfg_ver", kConfigVersion);
    prefs.putBool("wiz_done", firstRunDone);
    prefs.putString("loc", locationName);
    prefs.putBool("en_hub", enableHub);
    prefs.putUShort("hb_s", heartbeatIntervalS);
    prefs.putUChar("log_lvl", logLevel);
    prefs.putBool("en_mdns", enableMdns);
    prefs.putUShort("wdt_s", watchdogS);
    prefs.putUShort("scan_s", wifiScanIntervalS);
    prefs.putUShort("ble_s", bleScanIntervalS);
    prefs.putBool("en_ble", bleEnabled);
    prefs.putString("known_ssid", knownSsids);
    prefs.putString("known_ble", knownBle);
    prefs.putChar("rssi_drop", rssiWarnDrop);
    prefs.putUChar("score_warn", rfScoreWarn);
    prefs.putUChar("cong_warn", congestionWarn);
    prefs.putString("mode", opMode);
    prefs.end();
}

void ConfigStore::factoryReset() {
    prefs.begin("esphub", false);
    prefs.clear();
    prefs.end();
    prefs.begin("rfmon", false);
    prefs.clear();
    prefs.end();
    prefs.begin("rfmon_loc", false);
    prefs.clear();
    prefs.end();
    applyDefaults();
}

void ConfigStore::toJson(JsonObject obj) const {
    obj["deviceName"] = deviceName;
    obj["locationName"] = locationName;
    obj["hubHost"] = hubHost;
    obj["hubPort"] = hubPort;
    obj["enableHub"] = enableHub;
    obj["heartbeatIntervalS"] = heartbeatIntervalS;
    obj["logLevel"] = logLevel;
    obj["enableMdns"] = enableMdns;
    obj["watchdogS"] = watchdogS;
    obj["wifiScanIntervalS"] = wifiScanIntervalS;
    obj["bleScanIntervalS"] = bleScanIntervalS;
    obj["bleEnabled"] = bleEnabled;
    obj["knownSsids"] = knownSsids;
    obj["knownBle"] = knownBle;
    obj["rssiWarnDrop"] = rssiWarnDrop;
    obj["rfScoreWarn"] = rfScoreWarn;
    obj["congestionWarn"] = congestionWarn;
    obj["opMode"] = opMode;
    obj["firstRunDone"] = firstRunDone;
    obj["board"] = RFMON_BOARD_ID;
    obj["boardLabel"] = RFMON_BOARD_LABEL;
}

static String jsonString(JsonVariantConst v, const String& fallback) {
    if (v.isNull()) return fallback;
    return v.as<String>();
}

bool ConfigStore::fromJson(JsonVariantConst obj) {
    if (!obj.is<JsonObjectConst>()) return false;
    deviceName = jsonString(obj["deviceName"], deviceName);
    locationName = jsonString(obj["locationName"], locationName);
    hubHost = jsonString(obj["hubHost"], hubHost);
    if (!obj["hubPort"].isNull()) hubPort = obj["hubPort"].as<int>();
    if (!obj["enableHub"].isNull()) enableHub = obj["enableHub"].as<bool>();
    if (!obj["heartbeatIntervalS"].isNull()) heartbeatIntervalS = obj["heartbeatIntervalS"].as<uint16_t>();
    if (!obj["logLevel"].isNull()) logLevel = obj["logLevel"].as<uint8_t>();
    if (!obj["enableMdns"].isNull()) enableMdns = obj["enableMdns"].as<bool>();
    if (!obj["watchdogS"].isNull()) watchdogS = obj["watchdogS"].as<uint16_t>();
    if (!obj["wifiScanIntervalS"].isNull()) wifiScanIntervalS = obj["wifiScanIntervalS"].as<uint16_t>();
    if (!obj["bleScanIntervalS"].isNull()) bleScanIntervalS = obj["bleScanIntervalS"].as<uint16_t>();
    if (!obj["bleEnabled"].isNull()) bleEnabled = obj["bleEnabled"].as<bool>();
    knownSsids = jsonString(obj["knownSsids"], knownSsids);
    knownBle = jsonString(obj["knownBle"], knownBle);
    if (!obj["rssiWarnDrop"].isNull()) rssiWarnDrop = obj["rssiWarnDrop"].as<int8_t>();
    if (!obj["rfScoreWarn"].isNull()) rfScoreWarn = obj["rfScoreWarn"].as<uint8_t>();
    if (!obj["congestionWarn"].isNull()) congestionWarn = obj["congestionWarn"].as<uint8_t>();
    opMode = jsonString(obj["opMode"], opMode);
    if (!obj["firstRunDone"].isNull()) firstRunDone = obj["firstRunDone"].as<bool>();
    if (heartbeatIntervalS < 5) heartbeatIntervalS = 5;
    if (wifiScanIntervalS < 8) wifiScanIntervalS = 8;
    if (bleScanIntervalS < 5) bleScanIntervalS = 5;
    applyModeIntervals();
    return true;
}
