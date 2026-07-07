# ESP32 OTA Setup

## Architecture

```
OTA/ota_upload.py   — Cross-platform OTA upload script
OTA/ota_upload.sh   — Bash wrapper for OTA upload
lib/OTA/OTA.h       — OTA library header
lib/OTA/OTA.cpp     — OTA implementation (ArduinoOTA)
activity6/partitions_ota.csv  — Partition table with OTA slots
activity6/platformio.ini      — Build envs (serial + OTA)
```

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

**Python script:**
```bash
cd activity6
pio run -e esp32dev
../OTA/ota_upload.py .pio/build/esp32dev/firmware.bin 192.168.x.x
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
