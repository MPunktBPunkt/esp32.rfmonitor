#include "EventLog.h"
#include "core/HwCaps.h"
#include "core/TimeBase.h"
#include <string.h>

void EventLog::begin() {
    capacity_ = HwCaps::eventCapacity();
    if (capacity_ < 8) capacity_ = 8;
    events_ = (AppEvent*)HwCaps::alloc(capacity_ * sizeof(AppEvent), true);
    if (events_) memset(events_, 0, capacity_ * sizeof(AppEvent));
    head_ = 0;
    count_ = 0;
    nextId_ = 1;
}

uint32_t EventLog::emit(uint8_t level, const char* type, const char* title, const char* details) {
    if (!events_ || capacity_ == 0) return 0;
    AppEvent e;
    e.id = nextId_++;
    e.ts = TimeBase::uptimeS();
    e.level = level;
    e.acked = false;
    e.pinned = false;
    e.ackedAtS = 0;
    strncpy(e.type, type ? type : "", sizeof(e.type) - 1);
    strncpy(e.title, title ? title : "", sizeof(e.title) - 1);
    strncpy(e.details, details ? details : "", sizeof(e.details) - 1);
    events_[head_] = e;
    head_ = (head_ + 1) % capacity_;
    if (count_ < capacity_) count_++;
    Serial.printf("[EVT] %s %s %s\n", e.type, e.title, e.details);
    return e.id;
}

const AppEvent& EventLog::at(size_t newestOffset) const {
    static AppEvent empty;
    if (!events_ || count_ == 0 || newestOffset >= count_) return empty;
    size_t idx = (head_ + capacity_ - 1 - newestOffset) % capacity_;
    return events_[idx];
}

AppEvent* EventLog::findMutable(uint32_t id) {
    if (!events_ || id == 0) return nullptr;
    for (size_t i = 0; i < count_; i++) {
        size_t idx = (head_ + capacity_ - 1 - i) % capacity_;
        if (events_[idx].id == id) return &events_[idx];
    }
    return nullptr;
}

bool EventLog::ack(uint32_t id) {
    AppEvent* e = findMutable(id);
    if (!e) return false;
    e->acked = true;
    e->ackedAtS = TimeBase::uptimeS();
    return true;
}

bool EventLog::pin(uint32_t id, bool on) {
    AppEvent* e = findMutable(id);
    if (!e) return false;
    e->pinned = on;
    return true;
}

void EventLog::clear() {
    if (!events_ || count_ == 0) {
        head_ = 0;
        count_ = 0;
        return;
    }
    // Preserve pinned entries (newest-first → rewrite ring oldest-first).
    AppEvent kept[16];
    size_t kn = 0;
    for (size_t i = 0; i < count_ && kn < 16; i++) {
        const AppEvent& e = at(i);
        if (e.pinned) kept[kn++] = e;
    }
    head_ = 0;
    count_ = 0;
    for (size_t i = kn; i > 0; i--) {
        events_[head_] = kept[i - 1];
        head_ = (head_ + 1) % capacity_;
        count_++;
    }
}

void EventLog::toJson(JsonArray arr, size_t maxItems, const char* typeFilter, int levelFilter,
                      bool pinnedOnly) const {
    if (maxItems == 0) maxItems = capacity_;
    size_t n = count_ < maxItems ? count_ : maxItems;
    for (size_t i = 0; i < n; i++) {
        const AppEvent& e = at(i);
        if (pinnedOnly && !e.pinned) continue;
        if (levelFilter >= 0 && e.level != (uint8_t)levelFilter) continue;
        if (typeFilter && typeFilter[0] && strcmp(e.type, typeFilter) != 0) continue;
        JsonObject o = arr.add<JsonObject>();
        o["id"] = e.id;
        o["ts"] = e.ts;
        o["level"] = e.level;
        o["type"] = e.type;
        o["title"] = e.title;
        o["details"] = e.details;
        o["acked"] = e.acked;
        o["pinned"] = e.pinned;
        o["ackedAtS"] = e.ackedAtS;
    }
}
