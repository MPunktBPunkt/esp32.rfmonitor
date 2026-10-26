# Motion Sensitivity Characterization

**Kein Presence-Feature.** Frage: *Kann der RFMonitor reproduzierbare kurzzeitige Änderungen des eigenen WLAN-Links (Router↔ESP) durch eine bewegte Person erfassen?*

## Warum nicht der Score-Pfad?

`RfAnalyzer::recompute()` läuft alle **4 s** und füttert ein **12er**-RSSI-Fenster (~48 s). Ein Walk-by von 1–2 s wird dort zu einer winzigen Delle/Jitter-Zahl — genau der 4‑dB-Hinweis aus dem ersten Versuch, ohne Zeitauflösung.

## Profil `motion`

| Parameter | Wert |
|-----------|------|
| Own-Link RSSI | alle **200 ms** → Ring Mini 900 / S3 1500 |
| WiFi-/BLE-Scan | praktisch pausiert (600 s / BLE aus) |
| Score | weiter alle 4 s (unverändert) |
| Phasen | **CAL** · **W1** · **W2** (optional) |

Export: `motionRssi.samples` = `[uptimeMs, rssi, phaseIdx, connected]`.  
Live-UI: `/api/session/motion?n=300` (letzte N Samples, kein Full-Export).  
Leichter Export: `/api/session/export?motion=0` (nur Phase-Aggregates).

## Protokoll (Standard)

| Phase | Dauer (empfohlen) | Aktion |
|-------|-------------------|--------|
| **CAL** | 45–60 s | Still — **eine** Ruhekalibrierung (σ / Mittelwert) |
| **W1** | 45–60 s | Am ESP vorbei und zurück |
| **W2** | 45–60 s | Optionaler zweiter Durchgang (Repeatability) |

**Eine Ruhephase reicht.** Excursion und SNR_motion beziehen sich auf CAL. Früheres R0/R2/R4 war nur Charakterisierungs-Robustheit (Drift zwischen Walk-bys) — für den Küchenlauf unnötig.

Legacy-IDs R0–R4 werden von `rf-motion-characterize` weiterhin akzeptiert.

## Auswertung

```bash
tools/rf-motion-characterize docs/evidence/.../mini-session.json \
                             docs/evidence/.../s3-session.json
```

Liefert u. a. Ruhe-σ (CAL), Walk-by max |Δ|, SNR_motion ≈ excursion/σ, Repeatability W1↔W2, Mini↔S3-Hinweis.

## Audit-Framing

- **FACT:** RSSI hat sich verändert / nicht verändert  
- **INFERENCE:** Bewegung beeinflusst den beobachteten WLAN-Pfad (wenn reproduzierbar)  
- **REJECTED:** „ESP erkennt Handy als WLAN-Sender“ — nicht diese Messklasse  
- **UNKNOWN:** Übertragbarkeit auf andere Räume/Geometrien  
- CSI / Promiscuous: bewusst **nicht** — erst wenn High-rate Own-Link scheitert
