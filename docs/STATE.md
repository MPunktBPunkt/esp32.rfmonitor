# Project State — esp32.rfmonitor

| | |
|--|--|
| Version | **1.8.0** (Channel + Multi-res History) |
| Lab | Mini `.87` + S3 `.88` |
| Evidence | [lab-baseline](evidence/2026-10-25-lab-baseline/) · [kitchen side-by-side](evidence/2026-10-25-kitchen-side-by-side/) |
| Hub | [HUB_CONTRACT.md](HUB_CONTRACT.md) |

## Erledigt bis 1.7

Observability → Score v2 → Hub v2 → Validation 1.6 → Mini BLE-Fix → AP Lifetime / Event ACK

## Erledigt in 1.8.0

- Channel History: Snaps alle 30 s (AP-Count/Kanal) + UI-Heatmap
- Multi-resolution History: Raw / 1 min / 15 min (`?res=`)
- Metric `channel_pressure` in History
- Baseline Drift: magnitude, z-scores, `elevated` + Finding

## Als Nächstes

**P1**
- [ ] Field Session Profile
- [ ] Mini↔S3 Characterization nach 1.6.1/1.8 erneut
- [ ] Controlled Score Characterization (A–G)

**P2**
- [ ] ioBroker Hub Contract v2
- [ ] Langzeit-Hub-History

## Bewusst zurückgestellt

OLED · starke BLE-Taxonomie · Interference-Score · 5 GHz auf Verdacht
