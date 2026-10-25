#include "SessionProfile.h"
#include <string.h>

namespace SessionProfile {
namespace {

const SessionProfileDef kProfiles[] = {
    // Reproducible stationary field run (~10 min).
    {"field", "Field Session", 600, 20, 15, true, false},
    // Controlled A–G experiments — duration manual; phases via API/UI.
    {"characterization", "Score Characterization", 0, 10, 8, true, true},
    // Short smoke / regression window.
    {"quick", "Quick Check", 180, 10, 8, true, false},
    // Aggressive instrumentation without fixed end.
    {"diagnose", "Diagnose", 0, 10, 8, true, false},
};

constexpr size_t kCount = sizeof(kProfiles) / sizeof(kProfiles[0]);

}  // namespace

const SessionProfileDef* find(const char* id) {
    if (!id || !id[0]) return &kProfiles[0];
    for (size_t i = 0; i < kCount; i++) {
        if (strcasecmp(kProfiles[i].id, id) == 0) return &kProfiles[i];
    }
    return nullptr;
}

const SessionProfileDef& field() { return kProfiles[0]; }
const SessionProfileDef& characterization() { return kProfiles[1]; }

void listToJson(JsonArray arr) {
    for (size_t i = 0; i < kCount; i++) {
        JsonObject o = arr.add<JsonObject>();
        o["id"] = kProfiles[i].id;
        o["title"] = kProfiles[i].title;
        o["durationS"] = kProfiles[i].durationS;
        o["wifiScanIntervalS"] = kProfiles[i].wifiScanIntervalS;
        o["bleScanIntervalS"] = kProfiles[i].bleScanIntervalS;
        o["bleEnabled"] = kProfiles[i].bleEnabled;
        o["characterization"] = kProfiles[i].characterization;
    }
}

}  // namespace SessionProfile
