# Hub Contract — rfmonitor v2 (ab FW 1.5)

`fwType` bleibt **`rfmonitor`**.

## Heartbeat-Erweiterung

Zusätzlich zu den klassischen Top-Level-Feldern (`mac`, `name`, `ios`, …):

```json
{
  "fwType": "rfmonitor",
  "location": "Technikraum",
  "rf": {
    "contract": "rfmonitor-hub-v2",
    "score": 93,
    "scoreVersion": 2,
    "scoreV1": 93,
    "scoreV2": 93,
    "measurementQuality": 100,
    "locationDeviation": -2,
    "channelPressure": 20,
    "apCount": 4,
    "bleCount": 11,
    "radioDutyPct": 18.5,
    "radioDuty5mPct": 8.2,
    "radioDuty1hPct": 12.4,
    "wifiRssi": -48,
    "locationValid": true,
    "alert": {
      "contract": "rfmonitor-hub-v2",
      "any": false,
      "flags": {
        "score_low": false,
        "channel_pressure_high": false,
        "data_quality_low": false,
        "radio_budget_high": false,
        "baseline_deviation": false,
        "wifi_disconnected": false
      }
    }
  }
}
```

## IO-Werte (Auswahl)

Bestehende Sensoren plus:

| Key | Bedeutung |
|-----|-----------|
| `rf_score_v1` / `rf_score_v2` | parallele Scores |
| `measurement_quality` | Datenqualität |
| `location_deviation` | Standort-Δ |
| `radio_duty_5m_pct` / `radio_duty_1h_pct` | Fenster-Duty (Alerts nutzen 5 m) |
| `alert_any` | 1 wenn irgendein Hysterese-Alert aktiv |
| `alert_score_low` / `alert_pressure_high` / `alert_quality_low` | Einzel-Flags |

Schwellwert-Logik/Hysterese liegt **on-device**; der Hub kann `rf.alert.any` bzw. die IO-Flags spiegeln.
