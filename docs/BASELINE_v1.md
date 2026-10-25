# Baseline Freeze — esp32.rfmonitor v1.1.0

Referenzstand vor dem Observability-Ausbau (Instrumentierung / Measurement Model).

| Feld | Wert |
|------|------|
| Firmware | `1.1.0` |
| Commit | `95314cd` |
| Datum Freeze | 2026-10-25 |
| Boards | `rfmonitor-mini`, `rfmonitor-s3` |
| Score | Heuristik v1, **nicht** standortkalibriert |
| Audit | `esp32-rfmonitor-2026-pilot` @ Standard `0.6.0` |

## Was eingefroren ist

- Score-Formel und Breakdown wie in [RF_SCORE.md](RF_SCORE.md) (v1)
- API-Keys: `rfScore`, `channelLoad`, `channelCongestion`, `apDensity`, Hub-IOs
- Scan-Scheduling: WLAN und BLE sequentiell (`App::tickScans`)
- UI: Lab-Look (Syne + IBM Plex Mono), Gauge, Heatmap, Charts, Event-Terminal, SSE
- History: Ringbuffer (`RFMON_HISTORY_SIZE` 72 Mini / 180 S3)

## Bewusste Grenzen (v1)

| Gemessen | Abgeleitet (Heuristik) | Nicht messbar auf ESP-Scan-Pfad |
|----------|------------------------|----------------------------------|
| AP-Count, RSSI, Kanal, BSSID | density, congestion, channel_load | Airtime / CCA busy |
| BLE Adv / Beacon-Hints | — | Noise floor, PHY retries |
| Eigenes WiFi.RSSI, Reconnects | stability, RF-Score | Packet loss, Throughput |

`channel_load` / `congestion` sind **Proxies aus AP-Dichte**, keine physikalische Kanalbelegung.

## Nachfolger

Ab `1.2.0`: Observation Model (`raw` / `derived` / `assessment`), Scan-Instrumentierung, `measurementQuality`, explizite Proxy-Kennzeichnung. Score-Formel bleibt vorerst v1 (`scoreVersion: 1`), bis Location Baseline + Feldserie vorliegen.
