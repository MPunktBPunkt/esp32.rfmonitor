# Kitchen idle baseline — 2026-10-25

Keine Bewegung in der Küche. Location-Baseline Mini+S3, plus S3 Diagnose-Session „Küche“.

Firmware zum Zeitpunkt der Aufnahme: **1.9.1**.

## Kurzfazit

Ruhiger Own-Link, gut als Rest-Referenz für Motion Sensitivity (R0/R2/R4).

| | Mini `.87` | S3 `.88` |
|--|------------|----------|
| Location samples | 73 | 76 |
| ownRssi mean / σ | −39.2 / **0.67 dB** | −32.0 / **0.26 dB** |
| ownRssi min…max | −40…−38 | −33…−31 |
| score mean / σ | 91.9 / 0.94 | 90.8 / 1.18 |
| deviation / drift↑ | 0 / nein | −2 / nein |
| Session | — | diagnose „Küche“, 119 Samples, RSSI σ 0.22 |

Mini: BLE-Baseline 0 (erwartbar bei Mini-BLE-Verhalten). S3 sieht ~3–4 BLE-Geräte; leichte Score-/AP-Wackler, kein elevated Drift.

## Dateien

- `*-location.json` — Location Profile v2 inkl. Distribution
- `*-status.json` — Snapshot
- `*-history.json` — Raw-History
- `s3-session.json` — laufende Diagnose-Session zum Snapshot-Zeitpunkt
