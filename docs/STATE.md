# Project State — esp32.rfmonitor

| | |
|--|--|
| Version | **1.9.1** (Phase aggregates + export stream + larger rings) |
| Lab | Mini `.87` + S3 `.88` |
| Evidence | [lab-baseline](evidence/2026-10-25-lab-baseline/) · [kitchen](evidence/2026-10-25-kitchen-side-by-side/) · [char A–G](evidence/2026-10-25-score-characterization/) |
| Hub | [HUB_CONTRACT.md](HUB_CONTRACT.md) |

## Erledigt in 1.9.1

- Per-Phase Running Aggregates im Session-Export (überleben Ring-Wrap)
- `NetUtil::sendJson` mit `measureJson`+`reserve` (kein Truncate-Streaming)
- Ring 180 (Mini) / 300 (S3); `rf-characterize` nutzt Aggregates
- Feld-Evidence A–G gepusht (FAIL wegen schwachem RF-Stimulus — erwartet)

## Erledigt in 1.9.0

- Session-Profile: `field` / `characterization` / `quick` / `diagnose`
- Auto-Stop bei fester Dauer; Scan-Intervalle temporär überschrieben
- Phasen A–G (`POST /api/session/phase`) + Export
- `tools/rf-characterize sim|session` + CI-Sim

## Als Nächstes

- [ ] Characterization-Nachlauf mit starkem Stimulus (45–60 s/Phase) auf 1.9.1
- [ ] Mini↔S3 Score-Nachlauf / Drift-Vergleich
- [ ] ioBroker Hub Contract v2 Consumer

## Bewusst zurückgestellt

OLED · starke BLE-Taxonomie · Interference-Score · 5 GHz auf Verdacht
