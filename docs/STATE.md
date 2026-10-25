# Project State — esp32.rfmonitor

| | |
|--|--|
| Version | **1.7.0** (AP Lifetime + Event ACK/Pin) |
| Lab | Mini `.87` + S3 `.88` |
| Evidence | [lab-baseline](evidence/2026-10-25-lab-baseline/) · [kitchen side-by-side](evidence/2026-10-25-kitchen-side-by-side/) |
| Hub | [HUB_CONTRACT.md](HUB_CONTRACT.md) |

## Erledigt bis 1.5

Observability → Location/Session → Score v2 → Operator-UI → Hub-Contract v2

## Erledigt in 1.6.x

- Score Unit-Tests + Replay
- Zeitbasis NTP / `timeValid`
- Radio Duty 5 m / 1 h / Lifetime
- Location Profile v2 (Varianz) + Deviation-Teile + Why-Score
- Kitchen side-by-side Evidence (Fritzbox-nah)
- **1.6.1:** BLE-Scan-Watchdog; Mini Dual-Core-Worker aus (NimBLE-Hang starvete WiFi)

## Erledigt in 1.7.0

- AP Lifetime: firstSeen / seenCount / NEW·PERSISTENT·FLAPPING·DISAPPEARED
- Alert Conditions: pending→active→acked→recovering + `/api/alerts/ack`
- Event ACK/Pin (`/api/events/ack`, `/api/events/pin`); Clear behält Pins

## Als Nächstes

**P1 Operator**
- [ ] Channel History
- [ ] Multi-resolution History
- [ ] Baseline-Drift-Metrik

**P0 / Feld**
- [ ] Controlled Score Characterization (A–G)
- [ ] Mini↔S3 nach 1.6.1 erneut (Duty/BLE-Hunger weg?)
- [ ] ioBroker Hub Contract v2

## Bewusst zurückgestellt

OLED · starke BLE-Taxonomie · Interference-Score · 5 GHz auf Verdacht
