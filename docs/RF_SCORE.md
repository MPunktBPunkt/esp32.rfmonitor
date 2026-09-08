# RF-Score v1 (heuristisch)

Wertebereich: **0 … 100**. Höher = bessere lokale Funklage bzw. bessere eigene WLAN-Qualität.

## Formel

```
score = 100
if rssi < -55:          score -= (-55 - rssi) * 1.2
if rssi_jitter > 8:     score -= (rssi_jitter - 8) * 2
score -= congestion / 4
score -= density / 6
score -= (100 - stability) / 3
if WiFi disconnected:   score -= 25
score = clamp(score, 0, 100)
```

## Anteile

| Anteil | Quelle | Bedeutung |
|--------|--------|-----------|
| `rssi` | `WiFi.RSSI()` des verbundenen Netzes | eigene Verbindungsqualität |
| `rssi_jitter` | Min/Max der letzten 12 RSSI-Proben | Schwankung |
| `congestion` | max. APs auf einem 2.4-GHz-Kanal × 10 (cap 80) | Kanalüberlastung |
| `density` | sichtbare APs × 4 (cap 80) | AP-Dichte |
| `stability` | 100 minus Reconnect-Strafen, erholt sich langsam | Verbindungsstabilität |

`channel_load` (belegte Kanäle × 8) wird gemeldet, geht in v1 nicht direkt in den Score ein.

Die Formel ist bewusst einfach und darf in späteren Versionen kalibriert werden.
