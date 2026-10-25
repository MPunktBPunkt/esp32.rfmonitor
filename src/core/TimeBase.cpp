#include "TimeBase.h"
#include <time.h>

namespace TimeBase {
namespace {

bool syncRequested_ = false;
bool valid_ = false;
unsigned long lastAttemptMs_ = 0;
uint8_t attempts_ = 0;

bool probe() {
    time_t now = time(nullptr);
    // Reject unset / 1970-ish clocks.
    if (now < 1700000000L) return false;
    valid_ = true;
    return true;
}

void requestSync() {
    // UTC; field evidence can convert. CET would need DST rules on-device.
    configTime(0, 0, "pool.ntp.org", "time.nist.gov", "time.google.com");
    syncRequested_ = true;
    lastAttemptMs_ = millis();
    attempts_++;
}

}  // namespace

void begin() {
    valid_ = false;
    syncRequested_ = false;
    attempts_ = 0;
}

void onWifiConnected() {
    requestSync();
}

void loop() {
    if (valid_) return;
    if (!syncRequested_) return;
    if (probe()) {
        Serial.printf("[TIME] NTP ok epoch=%lu\n", (unsigned long)epochS());
        return;
    }
    // Retry a few times with backoff; then leave timeValid=false.
    if (attempts_ >= 8) return;
    if (millis() - lastAttemptMs_ < 15000UL * attempts_) return;
    requestSync();
}

uint32_t uptimeS() {
    return millis() / 1000UL;
}

bool timeValid() {
    if (valid_) return true;
    return probe();
}

uint32_t epochS() {
    if (!timeValid()) return 0;
    return (uint32_t)time(nullptr);
}

void appendJson(JsonObject target) {
    target["uptimeS"] = uptimeS();
    target["timeValid"] = timeValid();
    target["timestampEpochS"] = epochS();
}

}  // namespace TimeBase
