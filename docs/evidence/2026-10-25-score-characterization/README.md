# Score Characterization — 2026-10-25

Orchestrated remote phase markers A–G on Mini + S3 (FW 1.9.0).
Hold 120s per phase. Physical RF changes were operator-driven during windows.

## Run

```bash
tools/rf-characterize session docs/evidence/2026-10-25-score-characterization/mini-session.json
tools/rf-characterize session docs/evidence/2026-10-25-score-characterization/s3-session.json
```

## Phases

| ID | Note |
|----|------|
| A | RSSI -20dB — Geräte etwas wegdrehen / Abstand zur Fritzbox |
| B | +5 APs — Hotspots/Nachbar-APs aktivieren |
| C | same channel pressure — APs auf gleichem Kanal |
| D | more channels occupied |
| E | WLAN disconnect kurz |
| F | stabilize RSSI |
| G | return to baseline |


## Results

Ring capacity (Mini 120 / S3 150) is smaller than full A–G hold (~228 / ~302 samples), so early phases rolled out of the export ring.

See `mini-characterize.txt` / `s3-characterize.txt`.

Mini: HTTP export initially failed (Content-Length set, body stalled); succeeded after temporarily slowing scans/disabling BLE.

## Operator notes

- Phase D: Mini HTTP hung (~6 min); catch-up markers applied when recovered.
- Physical RF stimulus was operator-driven; not all intended deltas may have been applied.
