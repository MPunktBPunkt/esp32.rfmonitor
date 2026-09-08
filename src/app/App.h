#pragma once

#include <ArduinoJson.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include "core/ConfigStore.h"
#include "core/EventLog.h"
#include "core/HistoryStore.h"
#include "core/HubClient.h"
#include "rf/RfScanner.h"
#include "rf/BleTracker.h"
#include "rf/RfAnalyzer.h"

class App {
public:
    static App& instance();

    ConfigStore config;
    EventLog events;
    HistoryStore history;
    HubClient hub;
    RfScanner scanner;
    BleTracker ble;
    RfAnalyzer analyzer;
    WebServer server{80};

    void begin();
    void loop();
    void buildStatusJson(JsonDocument& doc);
    void buildHeartbeat(JsonDocument& doc);
    String statusString();

private:
    App() {}
    void checkResetButton();
    void setupWifi();
    void setupWeb();
    void registerRoutes();
    void handleRoot();
    void handleWizard();
    void handleMain();
    void handleOtaUpload();
    void handleOtaUploadFinish();
    void handleEvents();
    void sseSend(const String& data);
    void tickScans();

    WiFiClient sseClient_;
    unsigned long lastSse_ = 0;
    unsigned long lastWifiCheck_ = 0;
    unsigned long lastHubNote_ = 0;
    bool restartPending_ = false;
    unsigned long restartAt_ = 0;
};
