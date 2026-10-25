# Project State — esp32.rfmonitor

| | |
|--|--|
| Version | **1.9.0** (Field Session Profiles + Characterization) |
| Lab | Mini `.87` + S3 `.88` |
| Evidence | [lab-baseline](evidence/2026-10-25-lab-baseline/) · [kitchen](evidence/2026-10-25-kitchen-side-by-side/) |
| Hub | [HUB_CONTRACT.md](HUB_CONTRACT.md) |

## Erledigt in 1.9.0

- Session-Profile: `field` / `characterization` / `quick` / `diagnose`
- Auto-Stop bei fester Dauer; Scan-Intervalle temporär überschrieben
- Phasen A–G (`POST /api/session/phase`) + Export
- `tools/rf-characterize sim|session` + CI-Sim

## Als Nächstes

- [ ] Feld-Characterization A–G an beiden Boards (Evidence-Pack)
- [ ] Mini↔S3 Characterization-Nachlauf
- [ ] ioBroker Hub Contract v2

## Bewusst zurückgestellt

OLED · starke BLE-Taxonomie · Interference-Score · 5 GHz auf Verdacht
