# Hardware-Nutzung

| Board | Env | Cores | PSRAM | History | AP/BLE Cap |
|-------|-----|-------|-------|---------|------------|
| ESP32 Mini D1 | `rfmonitor-mini` | 2 | nein (typisch) | 72 | 24 / 24 |
| ESP32-S3 (ohne PSRAM) | `rfmonitor-s3` | 2 | nein zur Laufzeit | 180 | 40 / 40 |
| ESP32-S3 + PSRAM | `rfmonitor-s3` | 2 | ja (`psramFound`) | **720** | **96 / 96** |

## Dual-Core

Wenn `RFMON_USE_DUAL_CORE=1` und `ESP.getChipCores() >= 2`:

- **Core 0:** FreeRTOS-Task `rfwork` — WLAN/BLE-Scan-Scheduler + Score-Recompute
- **Core 1:** Arduino `loop` — WebServer, SSE, Hub-Heartbeat

Fallback: alles im Loop-Core (z. B. Create fehlgeschlagen oder Flag aus).

Mini D1 hat zwei Kerne, aber knappen internen Heap — deshalb kleine festen Caps, **kein** `BOARD_HAS_PSRAM`.

## PSRAM (S3)

Lab-Board (`.88`): DevKitC-1 **N8R2** — 8 MiB Flash + **2 MiB Quad-PSRAM**.

`platformio.ini` daher:

- `-DBOARD_HAS_PSRAM`
- `board_build.arduino.memory_type = qio_qspi` (**nicht** `qio_opi` — das ist für OPI/N8R8)

Zur Laufzeit entscheidet `HwCaps` (`heap_caps` + `psramFound()`):

- PSRAM ok → History 720 / Events 128 / AP+BLE 96 in SPIRAM
- sonst → S3-Internal-Defaults (180 / 40 / 40)

Reine N8-Module ohne PSRAM booten weiterhin; Caps bleiben klein.

## API

`/api/system/hw` und Statusfeld `hw`:

```json
{
  "cores": 2,
  "dualCore": true,
  "workerRunning": true,
  "psram": true,
  "psramSize": 8388608,
  "psramFree": 7000000,
  "historyCapacity": 720,
  "maxAps": 96,
  "maxBle": 96
}
```
