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
4. **Klare physische Änderung** halten (empfohlen **45–60 s**, nicht 2 min — sonst Ring-Druck ohne Mehrwert):
   - A: Gerät deutlich wegdrehen / Abstand (Ziel ≈ −15…−20 dB RSSI)
   - B: ≥3–5 zusätzliche APs/Hotspots sichtbar
   - C: mehrere APs bewusst auf denselben Kanal
   - D: APs auf zusätzliche Kanäle verteilen
   - E: WLAN wirklich trennen (≥20 s), dann reconnect
   - F/G: ruhig nahe Baseline-Lage
5. Optional Snapshot `GET /api/rf/snapshot`.
6. Session stoppen → `GET /api/session/export` speichern.
7. Auswerten:

```bash
tools/rf-characterize session docs/evidence/.../mini-session.json
tools/rf-score-replay docs/evidence/.../mini-session.json
```

Schwache Stimulus (nur Position tippen / Gast-WLAN einmal an) reicht typischerweise **nicht** — Score bleibt flach → FAIL ist erwartbar.

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
| Ring | 180 (Mini) / 300 (S3) Samples |
| Phase-Aggregates | live (`n`, `avgScore`, `avgOwnRssi`, …) — überleben Ring-Wrap |

`tools/rf-characterize session` bevorzugt Phase-Aggregates; Ring-Samples sind Zusatz.

Weitere Profile: `field` (10 min, 20/15), `quick` (3 min), `diagnose`.

## Evidence-Ordner

```
docs/evidence/YYYY-MM-DD-score-characterization/
  README.md
  mini-session.json
  s3-session.json
  *-characterize.txt
  snapshots/…
```

Feld 2026-10-25: erster Lauf mit schwachem Stimulus + Ring-Wrap → FAIL (dokumentiert). Ab 1.9.1 Aggregates + größerer Ring.

## Exit-Kriterium

A–E: Feature-Abzug und Score in erwartete Richtung (Replay + Device).  
`tools/rf-characterize sim` grün. Kein Physik-Claim.
