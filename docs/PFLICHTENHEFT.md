# Pflichtenheft `esp32.rfmonitor` v1.1

Eigenständiger ESP32-Knoten zur permanenten Beobachtung der lokalen Funkumgebung. Baut auf `esp-hub-base` auf, ist **kein** Teil von `esp32.toolnode`.

## Muss (v1.1)

- Basis: WiFiManager, Preferences, Web-UI, SSE, OTA, Heartbeat, Watchdog
- WLAN-Scan, BLE-Scan, generische Beacon-Anzeige
- RF-Score 0–100 (dokumentierte Heuristik) inkl. Breakdown
- RSSI-/Score-/AP-/BLE-History als Ringbuffer
- Lab-Web-UI mit Gauge, Heatmap, Charts, Event-Terminal
- Eventlog und Hub-IO-Metriken
- Config-Seite (kein Setup-Wizard)

## Kann später

- OLED-Seiten
- detaillierte Beacon-Typen
- Presence-Heuristik
- Langzeitaggregation außerhalb des Hub

## Struktur

- `App` — WLAN, Web, Heartbeat, Scan-Scheduler
- `RfScanner` — asynchroner WLAN-Scan
- `BleTracker` — NimBLE Advertisements
- `RfAnalyzer` — Score, Jitter, Events
- `HistoryStore` / `EventLog` / `ConfigStore` / `HubClient`
