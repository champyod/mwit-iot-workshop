# ESP32 OTA Setup

## Architecture

```
OTA/.env              — OTA target IP (not committed)
OTA/ota_upload.py     — Cross-platform OTA upload script
OTA/ota_upload.sh     — Bash wrapper for OTA upload
lib/OTA/OTA.h         — OTA library header
lib/OTA/OTA.cpp       — OTA implementation (ArduinoOTA)
activity6/partitions_ota.csv  — Partition table with OTA slots
activity6/platformio.ini      — Build envs (serial + OTA)
```

## Configuration (`.env`)

OTA target IP is stored in `OTA/.env` — not hardcoded in scripts. Edit for each device:

```bash
# OTA/.env
# Old device: 10.16.41.95
OTA_HOST=10.16.40.213
OTA_PORT=3232
```

CLI argument `python ota_upload.py firmware.bin 10.x.x.x` overrides `.env`.

## First Flash (via USB)

Must flash via USB once to upload bootloader + partition table + initial firmware:

```bash
cd activity6
pio run -e esp32dev -t upload
pio device monitor
```

Check serial output for IP address: `[OTA] Ready — http://192.168.x.x/update`

## OTA Update

**Browser method:** Open `http://<esp32-ip>/update`, select `.pio/build/esp32dev/firmware.bin`, upload.

**Python script (IP from `OTA/.env`):**
```bash
cd activity6
pio run -e esp32dev
../OTA/ota_upload.py .pio/build/esp32dev/firmware.bin
```

**PIO OTA env (after filling upload_port in platformio.ini):**
```bash
pio run -e esp32dev_ota -t upload
```

## Partition Layout

| Partition | Size   | Purpose          |
|-----------|--------|------------------|
| nvs       | 20KB   | WiFi config etc  |
| otadata   | 8KB    | OTA boot info    |
| app0      | 1.5MB  | Active firmware  |
| app1      | 1.5MB  | OTA update slot  |
| spiffs    | 960KB  | SPIFFS storage   |

## Requirements
- ESP32 with 4MB flash (standard)
- First upload always via USB
- ESP32 must be on same WiFi network as uploader

## Quick Reference

All commands from the OTA workflow directory:

### 1. Build firmware
```bash
cd <project>            # e.g. OTA/ or activity6/
~/.platformio/penv/bin/platformio run -e esp32dev
```

### 2. First flash via USB (bootstrap)
```bash
cd OTA/
~/.platformio/penv/bin/platformio run -e esp32dev -t upload
```

### 3. Net monitor (telnet log)
```bash
nc ${OTA_HOST:-10.16.40.213} 2323
```

### 4. OTA upload (after build)

Reads host from `OTA/.env` — no IP needed on CLI:
```bash
cd <project>
python3 ../OTA/ota_upload.py .pio/build/esp32dev/firmware.bin
```

Override with explicit IP (takes priority over `.env`):
```bash
python3 ../OTA/ota_upload.py .pio/build/esp32dev/firmware.bin 10.16.40.213
```

### Full workflow in one go
```bash
~/.platformio/penv/bin/platformio run -e esp32dev && \
python3 ../OTA/ota_upload.py .pio/build/esp32dev/firmware.bin
```
