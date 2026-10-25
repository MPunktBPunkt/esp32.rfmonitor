#include "App.h"
#include "core/HwCaps.h"
#include "core/NetUtil.h"
#include "core/TimeBase.h"
#include "web/UiPages.h"
#include <WiFi.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <esp_system.h>

static App* g_app = nullptr;

static void radioWorkerTask(void* arg) {
    App* app = static_cast<App*>(arg);
    for (;;) {
        app->radioTick();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

App& App::instance() {
    static App app;
    return app;
}

void App::begin() {
    g_app = this;
    Serial.begin(115200);
    delay(300);
    Serial.printf("\n=== esp32.rfmonitor v%s (%s) ===\n", FW_VERSION, RFMON_BOARD_LABEL);
    Serial.printf("[BOOT] reset reason %d\n", (int)esp_reset_reason());

    HwCaps::begin();
    TimeBase::begin();
    config.begin();
    events.begin();
    history.begin();
    checkResetButton();
    setupWifi();
    TimeBase::onWifiConnected();

    scanner.begin(&config, &events, &history);
    ble.begin(&config, &events, &history);
    analyzer.begin(&config, &events, &history, &scanner, &ble);

    if (config.enableMdns) {
        String mdns = "rfmon-" + NetUtil::macNoColon().substring(6);
        if (MDNS.begin(mdns.c_str())) Serial.printf("[mDNS] %s.local\n", mdns.c_str());
    }

    setupWeb();
    hub.begin(&config);
    hub.setPayloadBuilder([](JsonDocument& doc) { App::instance().buildHeartbeat(doc); });
    startRadioWorker();
    events.info("system.boot", "RF Monitor gestartet", FW_VERSION);
    if (config.enableHub) hub.sendNow();
}

void App::startRadioWorker() {
    radioOnWorker_ = false;
    if (!HwCaps::dualCore()) {
        Serial.println("[HW] RF worker: loop-core (single/disabled)");
        return;
    }
    // Pin RF scan/analyze to core 0; Arduino loop (Web/Hub) stays on core 1.
    BaseType_t ok = xTaskCreatePinnedToCore(
        radioWorkerTask, "rfwork", 8192, this, 1, &radioTask_, 0);
    if (ok == pdPASS) {
        radioOnWorker_ = true;
        HwCaps::setWorkerRunning(true);
        Serial.println("[HW] RF worker on core 0");
    } else {
        Serial.println("[HW] RF worker create failed — fallback loop-core");
    }
}

void App::radioTick() {
    tickScans();
    analyzer.loop();
}

void App::checkResetButton() {
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
    if (digitalRead(RESET_BUTTON_PIN) == HIGH) return;
    Serial.printf("[BOOT] Reset-Taste, halte %ds...\n", RESET_HOLD_SEC);
    unsigned long t = millis();
    while (digitalRead(RESET_BUTTON_PIN) == LOW) {
        if (millis() - t > (unsigned long)RESET_HOLD_SEC * 1000UL) {
            WiFiManager wm;
            wm.resetSettings();
            config.factoryReset();
            delay(400);
            ESP.restart();
        }
        delay(50);
    }
}

void App::setupWifi() {
    WiFi.mode(WIFI_STA);
    WiFiManager wm;
    WiFiManagerParameter pName("name", "Geraetename", config.deviceName.c_str(), 32);
    WiFiManagerParameter pHost("hub_host", "ESP-Hub IP", config.hubHost.c_str(), 40);
    WiFiManagerParameter pPort("hub_port", "Port", String(config.hubPort).c_str(), 6);
    wm.addParameter(&pName);
    wm.addParameter(&pHost);
    wm.addParameter(&pPort);
    wm.setConfigPortalTimeout(WIFI_PORTAL_TIMEOUT_S);
    wm.setAPCallback([](WiFiManager*) {
        Serial.println("[WiFi] Portal: " WIFI_AP_NAME);
    });
    if (!wm.autoConnect(WIFI_AP_NAME)) {
        delay(800);
        ESP.restart();
    }
    config.deviceName = String(pName.getValue());
    config.hubHost = String(pHost.getValue());
    config.hubPort = String(pPort.getValue()).toInt();
    config.save();
    Serial.printf("[WiFi] IP %s  Hub %s:%d\n", NetUtil::localIp().c_str(), config.hubHost.c_str(), config.hubPort);
}

void App::setupWeb() {
    registerRoutes();
    server.onNotFound([this]() {
        server.sendHeader(F("Location"), F("/"), true);
        server.send(302, F("text/plain"), F(""));
    });
    server.begin();
    Serial.printf("[WEB] http://%s/\n", NetUtil::localIp().c_str());
}

void App::buildStatusJson(JsonDocument& doc) {
    doc["name"] = config.deviceName;
    doc["location"] = config.locationName;
    doc["chip"] = NetUtil::chipModel();
    doc["board"] = RFMON_BOARD_ID;
    doc["boardLabel"] = RFMON_BOARD_LABEL;
    doc["mac"] = NetUtil::macNoColon();
    doc["ip"] = NetUtil::localIp();
    doc["rssi"] = WiFi.RSSI();
    doc["ssid"] = WiFi.SSID();
    doc["channel"] = WiFi.channel();
    doc["uptime"] = NetUtil::fmtUptime(TimeBase::uptimeS());
    doc["uptimeS"] = TimeBase::uptimeS();
    TimeBase::appendJson(doc["time"].to<JsonObject>());
    doc["heap"] = ESP.getFreeHeap();
    doc["sketch"] = ESP.getFreeSketchSpace();
    doc["hub"] = config.hubHost + ":" + String(config.hubPort);
    doc["interval"] = config.heartbeatIntervalS;
    doc["version"] = FW_VERSION;
    doc["hubOk"] = hub.lastOk();
    doc["hubEnabled"] = config.enableHub;
    doc["firstRunDone"] = config.firstRunDone;
    doc["opMode"] = config.opMode;
    doc["bleEnabled"] = config.bleEnabled;
    HwCaps::appendJson(doc["hw"].to<JsonObject>());
    analyzer.appendStatusJson(doc["rf"].to<JsonObject>());
    JsonObject ios = doc["ios"].to<JsonObject>();
    analyzer.appendIoValues(ios);
}

void App::buildHeartbeat(JsonDocument& doc) {
    doc["mac"] = NetUtil::macNoColon();
    doc["name"] = config.deviceName;
    doc["hwType"] = RFMON_HW_TYPE;
    doc["chipModel"] = NetUtil::chipModel();
    doc["version"] = FW_VERSION;
    doc["ip"] = NetUtil::localIp();
    doc["rssi"] = WiFi.RSSI();
    doc["uptime"] = TimeBase::uptimeS();
    TimeBase::appendJson(doc["time"].to<JsonObject>());
    doc["freeHeap"] = ESP.getFreeHeap();
    doc["freeSketch"] = ESP.getFreeSketchSpace();
    doc["fwType"] = "rfmonitor";
    doc["board"] = RFMON_BOARD_ID;
    doc["location"] = config.locationName;
    analyzer.appendHubContract(doc["rf"].to<JsonObject>());
    JsonObject ios = doc["ios"].to<JsonObject>();
    analyzer.appendIoValues(ios);
}

String App::statusString() {
    JsonDocument doc;
    buildStatusJson(doc);
    return NetUtil::jsonToString(doc);
}

void App::sseSend(const String& data) {
    if (!sseClient_ || !sseClient_.connected()) return;
    sseClient_.print("data: ");
    sseClient_.print(data);
    sseClient_.print("\n\n");
    sseClient_.flush();
}

void App::handleEvents() {
    if (sseClient_ && sseClient_.connected()) sseClient_.stop();
    sseClient_ = server.client();
    sseClient_.print(F("HTTP/1.1 200 OK\r\n"
                       "Content-Type: text/event-stream\r\n"
                       "Cache-Control: no-cache\r\n"
                       "Connection: keep-alive\r\n"
                       "Access-Control-Allow-Origin: *\r\n\r\n"));
    sseClient_.flush();
    sseSend(statusString());
}

void App::handleMain() {
    server.send_P(200, "text/html", PAGE_MAIN);
}

void App::handleRoot() {
    handleMain();
}

void App::handleOtaUpload() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("[OTA] Start %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) Update.printError(Serial);
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) Serial.printf("[OTA] OK %u Bytes\n", upload.totalSize);
        else Update.printError(Serial);
    }
}

void App::handleOtaUploadFinish() {
    if (Update.hasError()) server.send(500, F("text/plain"), F("OTA fehlgeschlagen!"));
    else server.send(200, F("text/plain"), F("OK - Neustart..."));
    delay(400);
    ESP.restart();
}

void App::registerRoutes() {
    server.on("/", HTTP_GET, [this]() { handleRoot(); });
    server.on("/rf", HTTP_GET, [this]() { handleMain(); });
    server.on("/wifi", HTTP_GET, [this]() { handleMain(); });
    server.on("/ble", HTTP_GET, [this]() { handleMain(); });
    server.on("/history", HTTP_GET, [this]() { handleMain(); });
    server.on("/events", HTTP_GET, [this]() { handleEvents(); });
    server.on("/eventlog", HTTP_GET, [this]() { handleMain(); });
    server.on("/config", HTTP_GET, [this]() { handleMain(); });
    server.on("/ota", HTTP_GET, [this]() { handleMain(); });
    server.on("/ota-upload", HTTP_POST, [this]() { handleOtaUploadFinish(); }, [this]() { handleOtaUpload(); });

    server.on("/api/status", HTTP_GET, [this]() {
        JsonDocument doc;
        buildStatusJson(doc);
        NetUtil::sendJson(server, 200, doc);
    });
    server.on("/api/rf/status", HTTP_GET, [this]() {
        JsonDocument doc;
        analyzer.appendStatusJson(doc.to<JsonObject>());
        doc["mode"] = config.opMode;
        doc["scanningWifi"] = scanner.busy();
        doc["scanningBle"] = ble.busy();
        NetUtil::sendJson(server, 200, doc);
    });
    server.on("/api/rf/observations", HTTP_GET, [this]() {
        JsonDocument doc;
        analyzer.appendObservationJson(doc.to<JsonObject>());
        HwCaps::appendJson(doc["hw"].to<JsonObject>());
        NetUtil::sendJson(server, 200, doc);
    });
    server.on("/api/rf/snapshot", HTTP_GET, [this]() {
        JsonDocument doc;
        analyzer.appendSnapshotJson(doc.to<JsonObject>());
        doc["name"] = config.deviceName;
        NetUtil::sendJson(server, 200, doc);
    });
    server.on("/api/rf/score", HTTP_GET, [this]() {
        JsonDocument doc;
        analyzer.appendStatusJson(doc.to<JsonObject>());
        NetUtil::sendJson(server, 200, doc);
    });

    server.on("/api/location/get", HTTP_GET, [this]() {
        JsonDocument doc;
        analyzer.location().appendJson(doc.to<JsonObject>());
        doc["locationName"] = config.locationName;
        NetUtil::sendJson(server, 200, doc);
    });
    server.on("/api/location/capture", HTTP_POST, [this]() {
        uint16_t dur = 300;
        String raw = server.arg("plain");
        if (raw.length()) {
            JsonDocument body;
            if (!deserializeJson(body, raw) && !body["durationS"].isNull()) {
                dur = body["durationS"].as<uint16_t>();
            }
        }
        JsonDocument resp;
        resp["ok"] = analyzer.location().startCapture(dur);
        resp["durationS"] = analyzer.location().captureTargetS();
        events.info("location.capture_start", "Standort-Baseline Aufnahme", String(dur).c_str());
        NetUtil::sendJson(server, 200, resp);
    });
    server.on("/api/location/capture/stop", HTTP_POST, [this]() {
        JsonDocument resp;
        resp["ok"] = analyzer.location().saveNowFromAverages();
        resp["valid"] = analyzer.location().valid();
        if (resp["ok"]) events.info("location.capture_done", "Standort-Baseline gespeichert", "");
        NetUtil::sendJson(server, 200, resp);
    });
    server.on("/api/location/capture/cancel", HTTP_POST, [this]() {
        analyzer.location().cancelCapture();
        JsonDocument resp;
        resp["ok"] = true;
        NetUtil::sendJson(server, 200, resp);
    });
    server.on("/api/location/clear", HTTP_POST, [this]() {
        analyzer.location().clear();
        events.info("location.cleared", "Standort-Baseline geloescht", "");
        JsonDocument resp;
        resp["ok"] = true;
        NetUtil::sendJson(server, 200, resp);
    });

    server.on("/api/session/status", HTTP_GET, [this]() {
        JsonDocument doc;
        analyzer.session().appendStatusJson(doc.to<JsonObject>());
        NetUtil::sendJson(server, 200, doc);
    });
    server.on("/api/session/start", HTTP_POST, [this]() {
        String label;
        String raw = server.arg("plain");
        if (raw.length()) {
            JsonDocument body;
            if (!deserializeJson(body, raw)) label = body["label"] | "";
        }
        JsonDocument resp;
        resp["ok"] = analyzer.session().start(label.c_str());
        events.info("session.start", "Mess-Session gestartet", label.c_str());
        NetUtil::sendJson(server, 200, resp);
    });
    server.on("/api/session/stop", HTTP_POST, [this]() {
        JsonDocument resp;
        resp["ok"] = analyzer.session().stop();
        analyzer.session().appendStatusJson(resp["session"].to<JsonObject>());
        events.info("session.stop", "Mess-Session gestoppt", "");
        NetUtil::sendJson(server, 200, resp);
    });
    server.on("/api/session/export", HTTP_GET, [this]() {
        JsonDocument doc;
        analyzer.session().appendExportJson(doc.to<JsonObject>());
        doc["name"] = config.deviceName;
        doc["location"] = config.locationName;
        analyzer.location().appendJson(doc["locationProfile"].to<JsonObject>());
        NetUtil::sendJson(server, 200, doc);
    });
    server.on("/api/rf/history", HTTP_GET, [this]() {
        JsonDocument doc;
        history.toJson(HistoryStore::RSSI, doc["rssi"].to<JsonArray>());
        history.toJson(HistoryStore::RF_SCORE, doc["rf_score"].to<JsonArray>());
        history.toJson(HistoryStore::AP_COUNT, doc["ap_count"].to<JsonArray>());
        history.toJson(HistoryStore::BLE_COUNT, doc["ble_count"].to<JsonArray>());
        history.toJson(HistoryStore::DATA_QUALITY, doc["data_quality"].to<JsonArray>());
        doc["capacity"] = (uint32_t)history.capacity();
        NetUtil::sendJson(server, 200, doc);
    });
    server.on("/api/wifi/list", HTTP_GET, [this]() {
        JsonDocument doc;
        doc["count"] = scanner.apCount();
        doc["capacity"] = (uint32_t)scanner.capacity();
        doc["lastScanTs"] = scanner.lastScanTs();
        doc["lastScanDurationMs"] = scanner.lastScanDurationMs();
        doc["failRatePct"] = scanner.failRatePct();
        scanner.resultsToJson(doc["aps"].to<JsonArray>());
        NetUtil::sendJson(server, 200, doc);
    });
    server.on("/api/wifi/channels", HTTP_GET, [this]() {
        JsonDocument doc;
        scanner.channelsToJson(doc["channels"].to<JsonArray>());
        NetUtil::sendJson(server, 200, doc);
    });
    auto wifiScan = [this]() {
        ble.stop();
        JsonDocument resp;
        resp["ok"] = scanner.requestScan();
        resp["scanning"] = scanner.busy();
        NetUtil::sendJson(server, 200, resp);
    };
    server.on("/api/wifi/scan", HTTP_GET, wifiScan);
    server.on("/api/wifi/scan", HTTP_POST, wifiScan);

    server.on("/api/ble/list", HTTP_GET, [this]() {
        JsonDocument doc;
        doc["count"] = ble.deviceCount();
        doc["capacity"] = (uint32_t)ble.capacity();
        doc["lastScanDurationMs"] = ble.lastScanDurationMs();
        doc["failRatePct"] = ble.failRatePct();
        ble.resultsToJson(doc["devices"].to<JsonArray>(), false);
        NetUtil::sendJson(server, 200, doc);
    });
    server.on("/api/ble/beacons", HTTP_GET, [this]() {
        JsonDocument doc;
        doc["count"] = ble.beaconCount();
        ble.resultsToJson(doc["beacons"].to<JsonArray>(), true);
        NetUtil::sendJson(server, 200, doc);
    });
    auto bleScan = [this]() {
        JsonDocument resp;
        resp["ok"] = ble.startWindow();
        resp["scanning"] = ble.busy();
        NetUtil::sendJson(server, 200, resp);
    };
    server.on("/api/ble/scan", HTTP_GET, bleScan);
    server.on("/api/ble/scan", HTTP_POST, bleScan);

    server.on("/api/history/rssi", HTTP_GET, [this]() {
        JsonDocument doc;
        history.toJson(HistoryStore::RSSI, doc["points"].to<JsonArray>());
        NetUtil::sendJson(server, 200, doc);
    });
    server.on("/api/history/rfscore", HTTP_GET, [this]() {
        JsonDocument doc;
        history.toJson(HistoryStore::RF_SCORE, doc["points"].to<JsonArray>());
        NetUtil::sendJson(server, 200, doc);
    });
    server.on("/api/history/apcount", HTTP_GET, [this]() {
        JsonDocument doc;
        history.toJson(HistoryStore::AP_COUNT, doc["points"].to<JsonArray>());
        NetUtil::sendJson(server, 200, doc);
    });
    server.on("/api/history/blecount", HTTP_GET, [this]() {
        JsonDocument doc;
        history.toJson(HistoryStore::BLE_COUNT, doc["points"].to<JsonArray>());
        NetUtil::sendJson(server, 200, doc);
    });
    server.on("/api/history/quality", HTTP_GET, [this]() {
        JsonDocument doc;
        history.toJson(HistoryStore::DATA_QUALITY, doc["points"].to<JsonArray>());
        NetUtil::sendJson(server, 200, doc);
    });

    server.on("/api/events/recent", HTTP_GET, [this]() {
        JsonDocument doc;
        String type = server.hasArg("type") ? server.arg("type") : "";
        int level = server.hasArg("level") ? server.arg("level").toInt() : -1;
        bool pinnedOnly = server.hasArg("pinned") && server.arg("pinned") == "1";
        events.toJson(doc["events"].to<JsonArray>(), events.capacity(),
                      type.length() ? type.c_str() : nullptr, level, pinnedOnly);
        doc["capacity"] = (uint32_t)events.capacity();
        NetUtil::sendJson(server, 200, doc);
    });
    server.on("/api/events/clear", HTTP_POST, [this]() {
        events.clear();
        JsonDocument resp;
        resp["ok"] = true;
        NetUtil::sendJson(server, 200, resp);
    });
    server.on("/api/events/ack", HTTP_POST, [this]() {
        JsonDocument body;
        if (!NetUtil::readJsonBody(server, body)) return;
        uint32_t id = body["id"].as<uint32_t>();
        JsonDocument resp;
        resp["ok"] = events.ack(id);
        NetUtil::sendJson(server, resp["ok"] ? 200 : 404, resp);
    });
    server.on("/api/events/pin", HTTP_POST, [this]() {
        JsonDocument body;
        if (!NetUtil::readJsonBody(server, body)) return;
        uint32_t id = body["id"].as<uint32_t>();
        bool on = body["pinned"].isNull() ? true : body["pinned"].as<bool>();
        JsonDocument resp;
        resp["ok"] = events.pin(id, on);
        NetUtil::sendJson(server, resp["ok"] ? 200 : 404, resp);
    });
    server.on("/api/alerts/ack", HTTP_POST, [this]() {
        JsonDocument body;
        if (!NetUtil::readJsonBody(server, body)) return;
        JsonDocument resp;
        if (!body["code"].isNull()) {
            String code = body["code"].as<String>();
            resp["ok"] = analyzer.ackAlert(code.c_str());
        } else {
            resp["ok"] = analyzer.ackAllAlerts();
        }
        analyzer.appendAlertState(resp["alert"].to<JsonObject>());
        NetUtil::sendJson(server, 200, resp);
    });

    server.on("/api/config/get", HTTP_GET, [this]() {
        JsonDocument doc;
        config.toJson(doc.to<JsonObject>());
        NetUtil::sendJson(server, 200, doc);
    });
    server.on("/api/config/save", HTTP_POST, [this]() {
        JsonDocument body;
        if (!NetUtil::readJsonBody(server, body)) return;
        config.fromJson(body.as<JsonVariantConst>());
        config.save();
        JsonDocument resp;
        resp["ok"] = true;
        NetUtil::sendJson(server, 200, resp);
    });
    server.on("/api/config/test", HTTP_POST, [this]() {
        JsonDocument resp;
        resp["ok"] = WiFi.status() == WL_CONNECTED;
        resp["wifi"] = WiFi.status() == WL_CONNECTED;
        resp["hub"] = !config.enableHub || hub.lastOk();
        resp["heap"] = ESP.getFreeHeap();
        resp["ble"] = ble.ready();
        resp["measurementQuality"] = analyzer.measurementQuality();
        resp["scoreVersion"] = analyzer.scoreVersion();
        HwCaps::appendJson(resp["hw"].to<JsonObject>());
        NetUtil::sendJson(server, 200, resp);
    });
    server.on("/api/system/restart", HTTP_POST, [this]() {
        JsonDocument resp;
        resp["ok"] = true;
        NetUtil::sendJson(server, 200, resp);
        restartPending_ = true;
        restartAt_ = millis() + 400;
    });
    server.on("/api/system/hw", HTTP_GET, [this]() {
        JsonDocument doc;
        HwCaps::appendJson(doc.to<JsonObject>());
        NetUtil::sendJson(server, 200, doc);
    });
}

void App::tickScans() {
    scanner.loop(false);
    ble.loop(false);  // includes BLE window watchdog
    if (scanner.busy()) {
        ble.stop();
        return;
    }
    // Prefer WiFi when due — a stuck BLE busy flag must not starve AP scans.
    if (scanner.due()) {
        if (ble.busy()) ble.stop();
        scanner.requestScan();
        return;
    }
    if (ble.busy()) return;
    if (ble.due()) ble.startWindow();
}

void App::loop() {
    server.handleClient();
    if (!radioOnWorker_) radioTick();
    hub.loop();
    TimeBase::loop();

    unsigned long now = millis();
    if (now - lastSse_ >= 4000UL) {
        lastSse_ = now;
        if (sseClient_ && sseClient_.connected()) sseSend(statusString());
    }
    if (now - lastWifiCheck_ >= 5000UL) {
        lastWifiCheck_ = now;
        if (WiFi.status() != WL_CONNECTED) WiFi.reconnect();
    }
    if (now - lastHubNote_ >= 8000UL) {
        lastHubNote_ = now;
        analyzer.noteHub(!config.enableHub || hub.lastOk());
    }
    if (config.enableHub && config.watchdogS > 0) {
        if (now - hub.lastSuccessMs() > (unsigned long)config.watchdogS * 1000UL) {
            Serial.println("[WDT] Hub timeout, Neustart");
            delay(300);
            ESP.restart();
        }
    }
    if (restartPending_ && now >= restartAt_) ESP.restart();
    delay(2);
}
