#include "EventLog.h"
#include <string.h>

void EventLog::begin() {
    head_ = 0;
    count_ = 0;
    nextId_ = 1;
}

uint32_t EventLog::emit(uint8_t level, const char* type, const char* title, const char* details) {
    AppEvent e;
    e.id = nextId_++;
    e.ts = millis() / 1000UL;
    e.level = level;
    strncpy(e.type, type ? type : "", sizeof(e.type) - 1);
    strncpy(e.title, title ? title : "", sizeof(e.title) - 1);
    strncpy(e.details, details ? details : "", sizeof(e.details) - 1);
    events_[head_] = e;
    head_ = (head_ + 1) % RFMON_EVENT_SIZE;
    if (count_ < RFMON_EVENT_SIZE) count_++;
    Serial.printf("[EVT] %s %s %s\n", e.type, e.title, e.details);
    return e.id;
}

const AppEvent& EventLog::at(size_t newestOffset) const {
    static AppEvent empty;
    if (count_ == 0 || newestOffset >= count_) return empty;
    size_t idx = (head_ + RFMON_EVENT_SIZE - 1 - newestOffset) % RFMON_EVENT_SIZE;
    return events_[idx];
}

void EventLog::clear() {
    head_ = 0;
    count_ = 0;
}

void EventLog::toJson(JsonArray arr, size_t maxItems, const char* typeFilter, int levelFilter) const {
    size_t n = count_ < maxItems ? count_ : maxItems;
    for (size_t i = 0; i < n; i++) {
        const AppEvent& e = at(i);
        if (levelFilter >= 0 && e.level != (uint8_t)levelFilter) continue;
        if (typeFilter && typeFilter[0] && strcmp(e.type, typeFilter) != 0) continue;
        JsonObject o = arr.add<JsonObject>();
        o["id"] = e.id;
        o["ts"] = e.ts;
        o["level"] = e.level;
        o["type"] = e.type;
        o["title"] = e.title;
        o["details"] = e.details;
    }
}
