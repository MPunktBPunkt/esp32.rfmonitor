# RF-Score (versioniert)

Wertebereich: **0 … 100**. Höher = bessere lokale Funklage / eigene WLAN-Qualität.

| Firmware | Aktive Assessment-Version | API |
|----------|---------------------------|-----|
| ≤ 1.3 | v1 | `scoreVersion: 1` |
| ≥ 1.4 | **v2** | `scoreVersion: 2`, zusätzlich `scoreV1` / `scoreV2` |

Beide Formeln laufen parallel (`src/rf/RfScore.cpp`) — gleiche Features, Replay möglich.  
Proxies ≠ Airtime. Separat: `measurementQuality`, `locationDeviation`.

---

## Features (Eingang)

| Feature | Quelle | Art |
|---------|--------|-----|
| `ownRssi` | `WiFi.RSSI()` | gemessen |
| `rssiJitter` | Min/Max der letzten 12 Proben | gemessen |
| `densityProxy` | AP-Count × 4 (cap 80) | **proxy** |
| `pressureProxy` | max APs/Kanal × 10 (cap 80) | **proxy** |
| `occupancyProxy` | belegte 2.4-Kanäle × 8 | **proxy** |
| `stability` | Reconnect-Erholung 0…100 | abgeleitet |
| `connected` | WiFi-Status | gemessen |
| `measurementQuality` | Scan-Alter/Fails/Duty | Meta |

---

## Score v1 (Referenz / Baseline Freeze)

```
score = 100
- ownLink:   rssi < -55 → (-55 - rssi) * 1.2
- jitter:    jitter > 8 → (jitter - 8) * 2
- pressure:  pressureProxy / 4
- density:   densityProxy / 6
- stability: (100 - stability) / 3
- disconnect: 25 wenn nicht verbunden
```

`occupancy` geht in v1 **nicht** ein.

---

## Score v2 (aktiv ab 1.4.0)

Ziel: dichtere Standorte weniger „bestrafen“, Occupancy leicht nutzen, unsichere Messungen markieren.

```
score = 100
- ownLink:     rssi < -50 → (-50 - rssi) * 1.0
- jitter:      jitter > 10 → (jitter - 10) * 1.5
- density:     densityProxy / 8
- pressure:    pressureProxy / 5
- occupancy:   occupancyProxy / 20
- stability:   (100 - stability) / 4
- disconnect:  20 wenn nicht verbunden
- qualityGate: +10 wenn DQ < 40, +5 wenn DQ < 60
```

Breakdown-Keys: `ownLink`, `jitter`, `density`, `pressure`, `occupancy`, `stability`, `disconnect`, `qualityGate`  
(Legacy-Aliasse `rssi`/`congestion` bleiben für UI.)

---

## Was der Score nicht ist

- Keine physikalische Interferenz / Airtime / CCA
- Kein Ersatz für `measurementQuality` oder Location Δ
- Standortkalibrierung läuft über **Location Profile**, nicht als Score-Offset
