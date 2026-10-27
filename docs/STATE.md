# Project State — esp32.rfmonitor

| | |
|--|--|
| Version | **1.10.5** (esp-hub-base Core A/B + gzip Web-UI) |
| Lab | Mini `.87` + S3 `.88` — beide 1.10.5 regressiert 2026-10-27 |
| Evidence | [lab](evidence/2026-10-25-lab-baseline/) · [kitchen](evidence/2026-10-25-kitchen-side-by-side/) · [idle](evidence/2026-10-25-kitchen-baseline-idle/) · [char A–G](evidence/2026-10-25-score-characterization/) · [motion](evidence/2026-10-26-kitchen-motion/) · [ruhe2](evidence/2026-10-26-kitchen-ruhe2/) · [walkby](evidence/2026-10-26-kitchen-walkby/) |
| Hub | [HUB_CONTRACT.md](HUB_CONTRACT.md) |

## Erledigt in 1.10.5 (Validierung Base 1.10.5)

- Shared Core unverändert sync (`HardwareInfo`, `CpuLoad` = Base)
- Web-UI: `web/index.html` → gzip-PROGMEM (`tools/pack_ui.py`, PIO pre-script)
- Labor: Web-OTA `.87`/`.88`; gzip-Header, Status-Keys, SSE, hwSummary/CPU OK

## Erledigt in 1.10.2

- Own-Link fein immer sichtbar (200 ms Ring) · Δ-dB-Zoom (±4 dB) · `/api/rf/ownlink`
- Toggle Δ / absolut; σ-Band + ±2 dB-Hilfslinien
- Küchen-Evidence: Ruhe #2 + Walk-by (max |Δ| ≈ 2 dB, SNR > 5)

## Erledigt in 1.10.1

- Live-Chart Own-Link Motion + `/api/session/motion`
- Phasen **CAL** · **W1** · **W2** optional (eine Ruhekalibrierung reicht)
- Export `?motion=0` ohne Sample-Ring
- `rf-motion-characterize` CAL/W1/W2 (+ Legacy R0–R4)

## Erledigt in 1.10.0

- Session-Profil **`motion`**: 200 ms Own-Link-RSSI, Scans pausiert
- Motion-Ring Mini 900 / S3 1500; Phase-Motion-Stats
- `tools/rf-motion-characterize` · [MOTION_SENSITIVITY.md](MOTION_SENSITIVITY.md)

## Erledigt in 1.9.1

- Per-Phase Aggregates (A–G überleben Ring-Wrap)
- Ring 180/300; `rf-characterize` nutzt Aggregates
- Feld-Evidence A–G (FAIL wegen schwachem Stimulus)

## Als Nächstes

- [ ] Mini↔S3 Score-Nachlauf / Drift
- [ ] ioBroker Hub Contract v2 Consumer
- [ ] Optional: Motion-Export streaming (große Ringe ohne HTTP-Stall)

## Bewusst zurückgestellt

CSI · Promiscuous · Presence-Claims · OLED · starke BLE-Taxonomie · 5 GHz
