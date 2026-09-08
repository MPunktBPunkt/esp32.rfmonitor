# esp32.rfmonitor

![Version](https://img.shields.io/badge/version-1.1.0-blue)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Build](https://github.com/MPunktBPunkt/esp32.rfmonitor/actions/workflows/build.yml/badge.svg)](https://github.com/MPunktBPunkt/esp32.rfmonitor/actions/workflows/build.yml)
[![Donate](https://img.shields.io/badge/Donate-PayPal-00457C.svg?logo=paypal)](https://www.paypal.com/donate/?business=martin%40bchmnn.de&currency_code=EUR)

> **ESP32 RF Environment Monitor** — permanenter WLAN-/BLE-Beobachter mit RSSI-Charts, RF-Score und ESP-Hub-Anbindung. Baut auf `esp-hub-base` auf, ist aber **kein** Teil von `esp32.toolnode`.

---

## Überblick

`esp32.rfmonitor` ist ein **eigenständiger, zweckgebundener Monitor-Node** für einen festen Standort (Technikraum, Flur, Büro, Werkstatt). Er scannt zyklisch die Funkumgebung, erkennt Veränderungen und zeigt Verläufe lokal an.

| Das ist es | Das ist es nicht |
|------------|------------------|
| RF-/WLAN-/BLE-Monitor | Spektrumanalysator |
| Infrastruktur-Scanner | Paket-Sniffer |
| Live-Status + History | Presence-System mit Raumgenauigkeit |
| Hub-Telemetrie | Ersatz für `toolnode` |

---

## Features v1.1

- WiFiManager-Setup (`ESP-RFMonitor-Setup`)
- Periodischer **WLAN-Scan** (SSID, BSSID, RSSI, Kanal, Auth, bekannt/unbekannt)
- Periodischer **BLE-Scan** (NimBLE) inkl. generischer Beacon-Erkennung (iBeacon / Eddystone)
- **RF-Score 0–100** aus RSSI, Jitter, Kanalüberlastung, AP-Dichte, Stabilität inkl. **Score-Breakdown**
- Ringbuffer-History: RSSI, RF-Score, AP-Count, BLE-Count
- Ereignislog (`wifi.new_ap`, `ble.new_device`, `rf.score_low`, …)
- Lab-Web-UI: Gauge, Kanal-Heatmap, Charts, Event-Terminal, SSE, Config (ohne Setup-Wizard)
- ESP-Hub Heartbeat inkl. RF-Metriken, Web-OTA und Hub-OTA
- Betriebsmodi: Live, Normal, Diagnose

---

## Hardware

| Board | PlatformIO-Env |
|-------|----------------|
| ESP32 Mini D1 | `rfmonitor-mini` |
| ESP32-S3 | `rfmonitor-s3` |

WLAN- und BLE-Scans laufen **nacheinander** (gemeinsames Radio).

---

## Quickstart

```bash
pio run -e rfmonitor-mini --target upload
pio device monitor
```

1. Hotspot `ESP-RFMonitor-Setup` → WLAN + Hub-IP
2. Browser: `http://<ESP-IP>/` → Monitor-UI (Config-Tab für Name/Intervalle)
3. Gerät erscheint im [ESP-Hub](https://github.com/MPunktBPunkt/iobroker.esp-hub)

---

## Hub-IO-Werte

`wifi_rssi`, `wifi_rssi_avg`, `wifi_rssi_jitter`, `rf_score`, `ap_count`, `ble_count`, `beacon_count`, `channel_load`, `scan_fail_count`

Heartbeat-Feld `fwType`: `rfmonitor`

---

## RF-Score

Dokumentiert in [docs/RF_SCORE.md](docs/RF_SCORE.md). Heuristik v1:

- Start 100
- Abzug bei eigenem RSSI unter −55 dBm
- Abzug bei RSSI-Jitter über 8 dB
- Abzug durch Kanalbelegung und AP-Dichte
- Abzug bei Reconnects / fehlender WLAN-Verbindung

---

## API (Auswahl)

| Endpoint | Funktion |
|----------|----------|
| `/api/status` | Gesamtstatus |
| `/api/rf/status` `/api/rf/score` `/api/rf/history` | RF |
| `/api/wifi/list` `/api/wifi/channels` `/api/wifi/scan` | WLAN |
| `/api/ble/list` `/api/ble/beacons` `/api/ble/scan` | BLE |
| `/api/history/rssi` … `/blecount` | Zeitreihen |
| `/api/events/recent` `/api/events/clear` | Events |
| `/api/config/get` `/save` `/test` | Config |
| `/events` | SSE |
| `/ota` `/ota-upload` | Web-OTA |

---

## Abgrenzung zu toolnode

`esp32.toolnode` ist die **modulare Labor-Plattform**. `esp32.rfmonitor` bleibt schlank: Scanner, Analyzer, History, Eventlog, UI, Hub-Telemetrie — ohne I²C/UART/SPI-Module.

---

## Lizenz

GPL-3.0 — siehe [LICENSE](LICENSE).
