# Score Characterization Protocol (A–G)

Ziel: nachweisen, dass Assessment **deterministisch auf seine Inputs** reagiert — nicht, dass RF-Physik verändert wurde.

## Host-Simulation (ohne Gerät)

```bash
tools/rf-characterize sim
```

Validiert A–G gegen `tools/rf_score.py` (gleiche Formeln wie Firmware). CI führt das mit aus.

## Feld-Protokoll (Gerät)

1. Baseline capturen (≥5 min, ruhig) — Location Profile.
2. Session starten mit Profil **`characterization`**:
   - UI: RF → Mess-Session → Profil „Score Characterization“
   - oder `POST /api/session/start` `{"profile":"characterization","label":"char-YYYY-MM-DD-mini"}`
3. Pro Experiment Phase setzen (UI-Buttons A–G oder API):
   - `POST /api/session/phase` `{"id":"A","note":"RSSI -20dB"}`
4. Änderung ≥2 min halten, optional Snapshot `GET /api/rf/snapshot`.
5. Session stoppen → `GET /api/session/export` speichern.
6. Auswerten:

```bash
tools/rf-characterize session docs/evidence/.../mini-session.json
tools/rf-score-replay docs/evidence/.../mini-session.json
```

### Experimenttabelle

| ID | Änderung | Erwartetes Feature | Score |
|----|----------|--------------------|-------|
| A | RSSI ≈ −20 dB | ownLink ↑ | ↓ |
| B | +≈5 fremde APs | density ↑ | ↓ |
| C | APs auf gleichem Kanal | pressure ↑ | ↓ |
| D | zusätzliche Kanäle | occupancy ↑ (v2) | ↓ |
| E | WLAN disconnect | disconnect | ↓ |
| F | RSSI stabilisieren | jitter ↓ | ↑ vs. Jitter-Lage |
| G | alles zurück | — | ≈ Baseline |

## Session-Profil `characterization`

| Parameter | Wert |
|-----------|------|
| WiFi-Scan | 10 s |
| BLE-Scan | 8 s |
| Dauer | manuell (Phasen) |
| Ring | 120 (Mini) / 150 (S3) Samples |

Weitere Profile: `field` (10 min, 20/15), `quick` (3 min), `diagnose`.

## Evidence-Ordner

```
docs/evidence/YYYY-MM-DD-score-characterization/
  README.md
  mini-session.json
  s3-session.json
  snapshots/A.json … G.json
```

## Exit-Kriterium

A–E: Feature-Abzug und Score in erwartete Richtung (Replay + Device).  
`tools/rf-characterize sim` grün. Kein Physik-Claim.
