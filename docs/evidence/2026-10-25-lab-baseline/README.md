# Lab Baseline Evidence — 2026-10-25

| | |
|--|--|
| Firmware während Capture | **1.3.0** |
| Dauer | 600 s |
| Geräte | Mini `192.168.178.87`, S3 `192.168.178.88` |
| Label Session | `feld-baseline-2026-10-25` |

## Ergebnis

| Board | Samples | Baseline Score | AP≈ | BLE≈ | ownRssi≈ |
|-------|---------|----------------|-----|------|----------|
| Mini | 151 | 96 | 3.0 | 0 | −48.3 |
| S3 | 151 | 93.4 | 3.8 | 11.6 | −31.9 |

Dateien: `*-location.json`, `*-session.json`, `*-snapshot.json`.

Hinweis: BLE-Count 0 auf Mini vs. ~12 auf S3 — unterschiedliche Antenne/Position oder Scan-Fenster; kein Kill, aber für Score-v2-Vergleich relevant.

Pilot Exit #5: Lab-Capture erfüllt Desk→Lab; echte Feldserie an weiterem Standort bleibt offen.
