# Location Profile & Sessions (ab 1.3.0, Varianz ab 1.6)

## Standort-Baseline

Statt eines simplen Score-Offsets speichert der Monitor ein **Standortnormal**.

`profileVersion` **2** (ab 1.6):

| Feld | Inhalt |
|------|--------|
| mean / stddev / min / max | Score, density, pressure, occupancy, AP/BLE count, own RSSI |
| sampleCount | Anzahl Capture-Samples |
| channels[1..14] | mittlere AP-Zahl pro Kanal |
| capturedAtUptimeS / capturedAtEpochS / capturedTimeValid | Zeitbasis |
| stale | true wenn Epoch-Alter ≥ 90 Tage |

Capture: `POST /api/location/capture` mit `{ "durationS": 300 }` (60…3600).  
Früh beenden: `POST /api/location/capture/stop` (≥3 Samples).  
NVS-Namespace: `rfmon_loc`.

API liefert weiterhin flache `baseline.*`-Mittelwerte (Kompatibilität) plus `distribution.*` mit Varianz.

### Location Deviation (−100…+100)

`deviationVersion` **1** — gewichtete Heuristik (noch nicht aus Characterization kalibriert):

| Anteil | Gewicht |
|--------|---------|
| Score-Δ | 45 % |
| Density | 15 % |
| Pressure | 15 % |
| AP count | 10 % |
| BLE count | 5 % |
| RSSI | 10 % |

Negativ = schlechter / dichter / schwächer als Normal.  
`deviationParts` bricht den Gesamtwert auf (Score, Density, Pressure, AP, BLE, RSSI).

Separat vom RF-Score (Absolute Quality vs. Standort-Δ).

## Measurement Session

`POST /api/session/start` → periodische Samples → `POST /api/session/stop` → `GET /api/session/export`.

Export enthält Firmware, Board, Score-Version, Summary, Sample-Ring (bis 90 Punkte) und Location-Profil.  
Ab 1.6 zusätzlich pro Sample: density, occupancy, jitter, stability, connected, optional `tsEpoch`.

Replay: `tools/rf-score-replay <session.json>`

## Coexistence Snapshot

`GET /api/rf/snapshot` — einmaliger Lage-Dump inkl. `time` (uptime/epoch/timeValid) und `why`.
