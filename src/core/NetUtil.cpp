#include "NetUtil.h"
#include <WiFi.h>

namespace NetUtil {

String macNoColon() {
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    mac.toUpperCase();
    return mac;
}

String localIp() {
    return WiFi.localIP().toString();
}

String fmtUptime(unsigned long seconds) {
    if (seconds < 60) return String(seconds) + "s";
    if (seconds < 3600) return String(seconds / 60) + "min " + String(seconds % 60) + "s";
    return String(seconds / 3600) + "h " + String((seconds % 3600) / 60) + "min";
}

String chipModel() {
    return String(ESP.getChipModel());
}

const char* levelName(uint8_t level) {
    switch (level) {
        case 0: return "ERROR";
        case 1: return "WARN";
        case 3: return "DEBUG";
        default: return "INFO";
    }
}

void addCors(WebServer& server) {
    server.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
    server.sendHeader(F("Access-Control-Allow-Headers"), F("Content-Type"));
}

String jsonToString(const JsonDocument& doc) {
    String out;
    const size_t n = measureJson(doc);
    if (n) out.reserve(n + 8);
    serializeJson(doc, out);
    return out;
}

void sendJson(WebServer& server, int code, const JsonDocument& doc) {
    addCors(server);
    // Prefer one contiguous String with reserve(): ESP32 WebServer's
    // setContentLength + empty send + raw client write truncates mid-body
    // on Mini under load. Phase aggregates keep exports useful even if a
    // full sample ring transfer stalls.
    server.send(code, F("application/json"), jsonToString(doc));
}

void sendError(WebServer& server, int code, const char* message) {
    JsonDocument doc;
    doc["ok"] = false;
    doc["error"] = message;
    sendJson(server, code, doc);
}

bool readJsonBody(WebServer& server, JsonDocument& doc) {
    String body = server.arg("plain");
    if (body.length() == 0) {
        sendError(server, 400, "leerer Body");
        return false;
    }
    if (deserializeJson(doc, body)) {
        sendError(server, 400, "ungueltiges JSON");
        return false;
    }
    return true;
}

}

