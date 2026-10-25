# esp32.rfmonitor

![Version](https://img.shields.io/badge/version-1.8.0-blue)
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

## Features v1.6

- WiFiManager-Setup (`ESP-RFMonitor-Setup`)
- Periodischer **WLAN-/BLE-Scan** (instrumentiert)
- **Observation Model** + **RF-Score v2** + Location/Session/Snapshot
- **Hub-Contract v2** (`rf.alert`, scoreV1/V2, Quality, Deviation) — [docs/HUB_CONTRACT.md](docs/HUB_CONTRACT.md)
- Location Profile **v2** (mean/stddev/min/max) + Deviation-Teile; NTP/`timeValid`
- Radio Duty **5 m / 1 h / Lifetime**; „Why this score?“ in API/UI
- Score Unit-Tests + Replay (`tools/rf-score-replay`)
- **Channel History** + Multi-res History (Raw / 1 min / 15 min)
- AP Lifetime · Event/Alert ACK/Pin · Baseline Drift
- Operator-UI: Timeline, Findings, Why?, History-Heatmap
- Event-Hysterese; PSRAM (S3) + Dual-Core RF-Worker (S3)
- SSE, Web-OTA, Betriebsmodi Live/Normal/Diagnose

---

## Hardware

| Board | PlatformIO-Env | Hinweis |
|-------|----------------|---------|
| ESP32 Mini D1 | `rfmonitor-mini` | 2 Cores, kein PSRAM → kleine Caps |
| ESP32-S3 | `rfmonitor-s3` | PSRAM wenn vorhanden → große Caps |

Details: [docs/HARDWARE.md](docs/HARDWARE.md). WLAN- und BLE-Scans laufen **nacheinander** (gemeinsames Radio).

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

`wifi_rssi`, `wifi_rssi_avg`, `wifi_rssi_jitter`, `rf_score`, `rf_score_version`, `measurement_quality`, `location_deviation`, `ap_count`, `ble_count`, `beacon_count`, `channel_load`, `channel_pressure_proxy`, `scan_fail_count`, `radio_duty_pct`, `radio_duty_5m_pct`, `radio_duty_1h_pct`

Heartbeat-Feld `fwType`: `rfmonitor`

---

## RF-Score & Observation

- Score: [docs/RF_SCORE.md](docs/RF_SCORE.md) (v1+v2 parallel, aktiv v2)
- Modell: [docs/OBSERVATION_MODEL.md](docs/OBSERVATION_MODEL.md)
- Characterization: [docs/SCORE_CHARACTERIZATION.md](docs/SCORE_CHARACTERIZATION.md)
- Stand: [docs/STATE.md](docs/STATE.md)

```bash
pio test -e native
python3 tools/test_rf_score.py
tools/rf-score-replay docs/evidence/2026-10-25-lab-baseline/mini-session.json
```

---

## API (Auswahl)

| Endpoint | Funktion |
|----------|----------|
| `/api/status` | Gesamtstatus inkl. `hw` + Observation |
| `/api/rf/status` `/observations` `/snapshot` `/history` | RF (`history?res=1m\|15m`) |
| `/api/rf/channels/history` | Channel-Druck über Zeit |
| `/api/location/get` `/capture` `/clear` | Standort-Baseline |
| `/api/session/start` `/stop` `/export` | Mess-Session |
| `/api/wifi/list` `/channels` `/scan` | WLAN |
| `/api/ble/list` `/beacons` `/scan` | BLE |
| `/api/system/hw` | Cores / PSRAM / Caps |
| `/api/config/get` `/save` `/test` | Config |
| `/events` | SSE |
| `/ota` `/ota-upload` | Web-OTA |

---

## Abgrenzung zu toolnode

`esp32.toolnode` ist die **modulare Labor-Plattform**. `esp32.rfmonitor` bleibt schlank: Scanner, Analyzer, History, Eventlog, UI, Hub-Telemetrie — ohne I²C/UART/SPI-Module.

---

## Lizenz

GPL-3.0 — siehe [LICENSE](LICENSE).
