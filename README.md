# esp32.rfmonitor

![Version](https://img.shields.io/badge/version-1.10.0-blue)
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

**Aktuell:** Firmware **1.10.0** · Lab Mini `192.168.178.87` · S3 `192.168.178.88` · Stand [docs/STATE.md](docs/STATE.md)

---

## Features (1.10.x)

- WiFiManager-Setup (`ESP-RFMonitor-Setup`)
- Periodischer **WLAN-/BLE-Scan** (instrumentiert; Mini single-core RF, S3 Dual-Core)
- **Observation → Derived → Assessment** + **RF-Score v1/v2** (aktiv v2)
- **Hub-Contract v2** (`rf.alert`, scoreV1/V2, Quality, Deviation) — [docs/HUB_CONTRACT.md](docs/HUB_CONTRACT.md)
- Location Profile **v2** (mean/stddev/min/max) + Deviation-Teile; NTP/`timeValid`
- Radio Duty **5 m / 1 h / Lifetime**; „Why this score?“ in API/UI
- **Channel History** + Multi-res History (Raw / 1 min / 15 min) + Baseline Drift
- **Field Session Profiles** (`field` / `characterization` / `motion` / `quick` / `diagnose`)
- Characterization-Phasen **A–G** inkl. **Phase-Aggregates** (überleben Ring-Wrap)
- **Motion Sensitivity**: 200 ms Own-Link-RSSI, Phasen **R0–R4** (kein Presence-Claim)
- Session-Ring Score **180/300**; Motion-RSSI **900/1500**
- AP Lifetime · Event/Alert ACK/Pin
- Operator-UI: Timeline, Findings, Why?, History-Heatmap
- SSE, Web-OTA, Betriebsmodi Live/Normal/Diagnose
- Host-Tools: Score-Tests, Replay, Characterization, Motion-Characterize

---

## Hardware

| Board | PlatformIO-Env | Hinweis |
|-------|----------------|---------|
| ESP32 Mini D1 | `rfmonitor-mini` | kein PSRAM → kleinere Caps; `RFMON_USE_DUAL_CORE=0` |
| ESP32-S3 | `rfmonitor-s3` | PSRAM → größere Caps; Dual-Core RF-Worker |

Details: [docs/HARDWARE.md](docs/HARDWARE.md). WLAN- und BLE-Scans laufen **nacheinander** (gemeinsames Radio).

---

## Quickstart

```bash
pio run -e rfmonitor-mini --target upload
pio device monitor
```

1. Hotspot `ESP-RFMonitor-Setup` → WLAN + Hub-IP  
2. Browser: `http://<ESP-IP>/` → Monitor-UI (Config-Tab für Name/Intervalle)  
3. Web-OTA: `/ota` bzw. `POST /ota-upload`  
4. Gerät erscheint im [ESP-Hub](https://github.com/MPunktBPunkt/iobroker.esp-hub)

---

## Hub-IO-Werte

`wifi_rssi`, `wifi_rssi_avg`, `wifi_rssi_jitter`, `rf_score`, `rf_score_v1`, `rf_score_v2`, `rf_score_version`, `measurement_quality`, `location_deviation`, `ap_count`, `ble_count`, `beacon_count`, `channel_load`, `channel_pressure_proxy`, `scan_fail_count`, `radio_duty_pct`, `radio_duty_5m_pct`, `radio_duty_1h_pct`, `alert_any`, …

Heartbeat: `fwType` = `rfmonitor`, Nested-Objekt `rf` nach [Hub Contract v2](docs/HUB_CONTRACT.md).

---

## RF-Score, Session & Evidence

| Doc | Inhalt |
|-----|--------|
| [docs/RF_SCORE.md](docs/RF_SCORE.md) | Score v1+v2 |
| [docs/OBSERVATION_MODEL.md](docs/OBSERVATION_MODEL.md) | Observation-Modell |
| [docs/LOCATION_PROFILE.md](docs/LOCATION_PROFILE.md) | Standort-Baseline |
| [docs/SCORE_CHARACTERIZATION.md](docs/SCORE_CHARACTERIZATION.md) | Protokoll A–G (stark stimulieren, 45–60 s/Phase) |
| [docs/MOTION_SENSITIVITY.md](docs/MOTION_SENSITIVITY.md) | Own-Link Walk-by / RF-Sensitivität (R0–R4) |
| [docs/STATE.md](docs/STATE.md) | Projektstand / Next |
| [docs/evidence/](docs/evidence/) | Lab-, Kitchen-, Characterization-Packs |

```bash
pio test -e native
python3 tools/test_rf_score.py
tools/rf-score-replay docs/evidence/2026-10-25-lab-baseline/mini-session.json
tools/rf-characterize sim
tools/rf-characterize session docs/evidence/2026-10-25-score-characterization/s3-session.json
tools/rf-motion-characterize docs/evidence/.../mini-session.json docs/evidence/.../s3-session.json
```

---

## API (Auswahl)

| Endpoint | Funktion |
|----------|----------|
| `/api/status` | Gesamtstatus inkl. `hw` + Observation |
| `/api/rf/status` `/observations` `/snapshot` `/score` | RF Live |
| `/api/rf/history?res=raw\|1m\|15m` | Multi-res History |
| `/api/rf/channels/history` | Channel-Druck über Zeit |
| `/api/location/get` `/capture` `/clear` | Standort-Baseline |
| `/api/session/profiles` `/start` `/phase` `/stop` `/export` | Mess-Session (Profile + Phasen A–G) |
| `/api/wifi/list` `/channels` `/scan` | WLAN |
| `/api/ble/list` `/beacons` `/scan` | BLE |
| `/api/events/recent` `/ack` `/pin` · `/api/alerts/ack` | Events / Alerts |
| `/api/system/hw` `/restart` | Hardware / Neustart |
| `/api/config/get` `/save` `/test` | Config |
| `/events` | SSE |
| `/ota` `/ota-upload` | Web-OTA |

---

## Abgrenzung zu toolnode

`esp32.toolnode` ist die **modulare Labor-Plattform**. `esp32.rfmonitor` bleibt schlank: Scanner, Analyzer, History, Eventlog, UI, Hub-Telemetrie — ohne I²C/UART/SPI-Module.

---

## Lizenz

GPL-3.0 — siehe [LICENSE](LICENSE).
