# Project State — esp32.rfmonitor

| | |
|--|--|
| Version | **1.10.0** (Motion Sensitivity session) |
| Lab | Mini `.87` + S3 `.88` |
| Evidence | [lab-baseline](evidence/2026-10-25-lab-baseline/) · [kitchen](evidence/2026-10-25-kitchen-side-by-side/) · [kitchen-idle](evidence/2026-10-25-kitchen-baseline-idle/) · [char A–G](evidence/2026-10-25-score-characterization/) |
| Hub | [HUB_CONTRACT.md](HUB_CONTRACT.md) |

## Erledigt in 1.10.0

- Session-Profil **`motion`**: 200 ms Own-Link-RSSI, Scans pausiert, Phasen R0–R4
- Motion-Ring Mini 900 / S3 1500; Phase-Motion-Stats im Export
- `tools/rf-motion-characterize` (σ, excursion, SNR, Repeatability, Mini↔S3)
- Doc: [MOTION_SENSITIVITY.md](MOTION_SENSITIVITY.md)

## Erledigt in 1.9.1

- Per-Phase Aggregates (A–G überleben Ring-Wrap)
- Ring 180/300; `rf-characterize` nutzt Aggregates
- Feld-Evidence A–G (FAIL wegen schwachem Stimulus)

## Als Nächstes

- [ ] Motion-Sensitivity-Lauf Mini+S3 (R0–R4), Evidence-Pack — Ruhe-σ Küche ok (Mini 0.67 / S3 0.26 dB)
- [ ] Mini↔S3 Score-Nachlauf / Drift
- [ ] ioBroker Hub Contract v2 Consumer

## Bewusst zurückgestellt

CSI · Promiscuous · Presence-Claims · OLED · starke BLE-Taxonomie · 5 GHz
