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
| Phasen | **R0–R4** |

Export: `motionRssi.samples` = `[uptimeMs, rssi, phaseIdx, connected]`.

## Protokoll

| Phase | Dauer (empfohlen) | Aktion |
|-------|-------------------|--------|
| R0 | 45–60 s | Still sitzen/stehen |
| R1 | 45–60 s | Am ESP vorbei und zurück |
| R2 | 45–60 s | Wieder still |
| R3 | 45–60 s | Zweiter Durchgang |
| R4 | 45–60 s | Still |

Optional später: Körper ohne Handy / Idle-Handy / Traffic-Handy trennen.

## Auswertung

```bash
tools/rf-motion-characterize docs/evidence/.../mini-session.json \
                             docs/evidence/.../s3-session.json
```

Liefert u. a. Rest-σ, Walk-by max |Δ|, SNR_motion ≈ excursion/σ, Repeatability R1↔R3, Mini↔S3-Hinweis (gemeinsam vs. pfadlokal).

## Audit-Framing

- **FACT:** RSSI hat sich verändert / nicht verändert  
- **INFERENCE:** Bewegung beeinflusst den beobachteten WLAN-Pfad (wenn reproduzierbar)  
- **REJECTED:** „ESP erkennt Handy als WLAN-Sender“ — nicht diese Messklasse  
- **UNKNOWN:** Übertragbarkeit auf andere Räume/Geometrien  
- CSI / Promiscuous: bewusst **nicht** — erst wenn High-rate Own-Link scheitert
