# Observation Model (ab 1.2.0)

Jede verdichtete Messung folgt drei Ebenen. Damit bleiben Rohdaten, Proxies und Score-Bewertung getrennt — Voraussetzung für Score-Replay und Audit-Evidence.

```text
OBSERVATION (raw)     → was der Radio-Pfad gesehen/getan hat
DERIVED (proxy)       → aus Zählungen/RSSI abgeleitete Kennzahlen
ASSESSMENT            → Score + Data Quality (kein Physik-Claim)
```

## JSON-Form (`/api/rf/status`, `/api/rf/observations`)

```json
{
  "observation": {
    "wifi": {
      "apCount": 17,
      "knownApCount": 3,
      "unknownApCount": 14,
      "strongestRssi": -41,
      "ownRssi": -61,
      "scanDurationMs": 1842,
      "scanAgeS": 12,
      "scanOk": true,
      "scanAttempts": 40,
      "scanFailures": 1
    },
    "ble": {
      "deviceCount": 12,
      "beaconCount": 4,
      "scanDurationMs": 4000,
      "scanAgeS": 8,
      "scanAttempts": 38,
      "scanFailures": 0
    },
    "radio": {
      "wifiBusyMs": 1842,
      "bleBusyMs": 4000,
      "dutyCyclePct": 14.2,
      "dutyLifetimePct": 14.2,
      "duty5mPct": 8.1,
      "duty1hPct": 11.0,
      "wifiScanning": false,
      "bleScanning": false
    },
    "link": {
      "rssiSampleCount": 12,
      "rssiAvg": -62.1,
      "rssiJitter": 5.0,
      "disconnects": 0,
      "connected": true
    }
  },
  "derived": {
    "density": { "value": 68, "kind": "proxy", "basis": "ap_count" },
    "channelPressure": { "value": 50, "kind": "proxy", "basis": "max_aps_per_channel" },
    "channelOccupancy": { "value": 42, "kind": "proxy", "basis": "occupied_2g_channels" }
  },
  "assessment": {
    "score": 71,
    "scoreVersion": 2,
    "scoreV1": 68,
    "scoreV2": 71,
    "scoreParts": { "ownLink": 7, "jitter": 0, "pressure": 10, "density": 8, "occupancy": 2, "stability": 0, "disconnect": 0, "qualityGate": 0 },
    "measurementQuality": 94,
    "locationDeviation": -3,
    "why": { "strongestPenalty": "pressure", "strongestPenaltyValue": 10 }
  },
  "time": { "uptimeS": 84231, "timeValid": true, "timestampEpochS": 1761402702 }
}
```

## Measurement Quality (0…100)

Separat vom RF-Score. Beantwortet: *Wie vertrauenswürdig ist die aktuelle Beobachtung?*

Abzüge (v1.2):

| Bedingung | Abzug |
|-----------|-------|
| Noch kein erfolgreicher WLAN-Scan | −40 |
| WLAN-Scan älter als 2× Intervall | −20 |
| WLAN Fail-Rate (Anteil) | ×0.4 |
| BLE aktiv und Fail-Rate | ×0.3 |
| RSSI-Fenster &lt; 6 Proben | −15 |
| Radio-Duty 5 m &gt; 40 % (Fallback: Lifetime) | −10 |

## Kompatibilität

Flache Keys (`rfScore`, `channelLoad`, `channelCongestion`, …) bleiben für UI/Hub v1 erhalten. Neue Clients sollen `observation` / `derived` / `assessment` nutzen.
