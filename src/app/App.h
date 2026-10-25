#pragma once

#include <ArduinoJson.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "core/ConfigStore.h"
#include "core/EventLog.h"
#include "core/HistoryStore.h"
#include "core/HubClient.h"
#include "rf/RfScanner.h"
#include "rf/BleTracker.h"
#include "rf/RfAnalyzer.h"
#include "rf/SessionProfile.h"

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
    void radioTick();
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
    void handleMain();
    void handleOtaUpload();
    void handleOtaUploadFinish();
    void handleEvents();
    void sseSend(const String& data);
    void tickScans();
    void startRadioWorker();
    void applySessionProfile(const SessionProfileDef& profile);
    void restoreScanIntervals();

    WiFiClient sseClient_;
    TaskHandle_t radioTask_ = nullptr;
    unsigned long lastSse_ = 0;
    unsigned long lastWifiCheck_ = 0;
    unsigned long lastHubNote_ = 0;
    bool restartPending_ = false;
    unsigned long restartAt_ = 0;
    bool radioOnWorker_ = false;
    // Saved scan intervals while a session profile is active.
    bool profileOverrides_ = false;
    uint16_t savedWifiIntervalS_ = 20;
    uint16_t savedBleIntervalS_ = 15;
    bool savedBleEnabled_ = true;
    String savedOpMode_ = "normal";
};
