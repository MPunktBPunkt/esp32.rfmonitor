#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "BuildFlags.h"

struct AppEvent {
    uint32_t id = 0;
    uint32_t ts = 0;
    uint8_t level = 2;
    char type[28] = {0};
    char title[48] = {0};
    char details[96] = {0};
};

class EventLog {
public:
    void begin();
    uint32_t emit(uint8_t level, const char* type, const char* title, const char* details = "");
    size_t count() const { return count_; }
    size_t capacity() const { return capacity_; }
    const AppEvent& at(size_t newestOffset) const;
    void clear();
    void toJson(JsonArray arr, size_t maxItems = 0, const char* typeFilter = nullptr, int levelFilter = -1) const;

    void error(const char* type, const char* title, const char* details = "") { emit(0, type, title, details); }
    void warn(const char* type, const char* title, const char* details = "") { emit(1, type, title, details); }
    void info(const char* type, const char* title, const char* details = "") { emit(2, type, title, details); }

private:
    AppEvent* events_ = nullptr;
    size_t capacity_ = RFMON_EVENT_SIZE;
    size_t head_ = 0;
    size_t count_ = 0;
    uint32_t nextId_ = 1;
};
