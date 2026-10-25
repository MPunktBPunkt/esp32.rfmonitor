# Kitchen Side-by-Side — 2026-10-25

Klassifikation: **`field-baseline` / hardware characterization**  
Ort: Küche · Geräte **nebeneinander**, wenige cm neben der Fritzbox · FW **1.6.0** (Capture)

| | Mini `.87` | S3 `.88` |
|--|------------|----------|
| Placement | side-by-side | side-by-side |
| own RSSI | ≈ −49 dBm | ≈ −32 dBm |
| AP count / channels | 4 · CH1/6/11 | 4 · CH1/6/11 (identisch) |
| Pressure / Occupancy | 20 / 24 | 20 / 24 |
| Score | 93 | 93 |
| BLE | ≈ 0–1 | ≈ 4 |
| Duty 5m (während Capture) | ≈ 0 % (Scan-Hunger) | ≈ 35 % |
| Baseline n | 76 | 76 |
| Session label | `Küche` | `Küche` |

## Befund

1. **Environment-Proxies board-übergreifend konsistent** (AP/Kanal/Score) — gutes Gate für das Assessment.
2. **RSSI Δ ≈ 17 dB** und **BLE Δ** sind Hardware-/Antennen-Gates, keine Aufstellungsdifferenz.
3. **Mini:** BLE-Fenster hing (`bleScanning` stuck) und blockierte WiFi-Scans → Duty≈0, DQ↓. Behoben in **1.6.1** (BLE-Timeout + Mini ohne Dual-Core-Worker).

## Dateien

`*-status.json`, `*-snapshot.json`, `*-location.json`, `*-session.json`, `summary.json`
