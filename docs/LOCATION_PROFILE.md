# Location Profile & Sessions (ab 1.3.0)

## Standort-Baseline

Statt eines simplen Score-Offsets speichert der Monitor ein **Standortnormal**:

| Feld | Bedeutung |
|------|-----------|
| score / density / channelPressure / occupancy | Mittelwerte während Capture |
| apCount / bleCount / ownRssi | Mittelwerte |
| channels[1..14] | mittlere AP-Zahl pro Kanal |

Capture: `POST /api/location/capture` mit `{ "durationS": 300 }` (60…3600).  
Früh beenden: `POST /api/location/capture/stop` (≥3 Samples).  
NVS-Namespace: `rfmon_loc`.

### Location Deviation (−100…+100)

Vergleicht aktuelle Beobachtung mit dem Profil. Negativ = schlechter / dichter / schwächer als Normal.

Separat vom RF-Score (Absolute Quality vs. Standort-Δ).

## Measurement Session

`POST /api/session/start` → periodische Samples → `POST /api/session/stop` → `GET /api/session/export`.

Export enthält Firmware, Board, Score-Version, Summary, Sample-Ring (bis 90 Punkte) und Location-Profil.

## Coexistence Snapshot

`GET /api/rf/snapshot` — einmaliger Lage-Dump (Kanäle, Score, Pressure-Proxy, BLE-Count, HW) für Vorher/Nachher-Vergleiche (z. B. BT-Gateway).
