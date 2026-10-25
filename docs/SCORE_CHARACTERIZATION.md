# Score Characterization Protocol (RFMON 1.6)

Ziel: nachweisen, dass Assessment **deterministisch auf seine Inputs** reagiert — nicht, dass RF-Physik verändert wurde.

## Vorbereitung

1. Standort-Baseline capturen (≥5 min, ruhige Umgebung).
2. Session starten: Label `char-YYYY-MM-DD-<board>`.
3. Nach jedem Experiment Snapshot (`GET /api/rf/snapshot`) + Session am Ende exportieren.
4. Replay: `tools/rf-score-replay docs/evidence/<pack>/<board>-session.json`

## Experimenttabelle

| ID | Änderung | Erwartetes Feature | Erwartete Score-Reaktion |
|----|----------|--------------------|--------------------------|
| A | RSSI ≈ −20 dB (Abstand/Orientierung) | ownLink ↑ | Score ↓ |
| B | +≈5 fremde APs (z. B. Hotspot-Cluster) | density ↑ | Score ↓ |
| C | Mehrere APs auf gleichem Kanal | pressure ↑ | Score ↓ |
| D | APs über zusätzliche Kanäle | occupancy ↑ | Score ↓ |
| E | WLAN disconnect (kurz) | disconnect | Score ↓ |
| F | RSSI stabilisieren (fix position) | jitter ↓ | Score ↑ Richtung Baseline |
| G | Alles zurück | — | Score ≈ Baseline |

Pro Experiment: ≥2 min halten, dann Recovery ≥2 min dokumentieren.

## Evidence-Ordner

```
docs/evidence/YYYY-MM-DD-score-characterization/
  README.md          # Tabelle + Beobachtungen
  mini-session.json
  s3-session.json    # optional parallel
  snapshots/A..G.json
```

## Exit-Kriterium

Für A–E: Feature-Abzug und Score bewegen sich in die erwartete Richtung (Replay und Device konsistent).  
Keine Behauptung über „echte Interferenz“ — nur Assessment-Sensitivität.
