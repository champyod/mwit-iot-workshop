# MWIT IoT MiniProject — Standalone Alarm

ESP32 alarm system for the MWIT IoT curriculum. Ultrasonic motion detection drives a relay siren and status LEDs, with a local web control panel. Network features (web panel, Discord alerts, Google Sheets logging) are best-effort enhancements — the alarm core works fully offline.

> Workshop activities live on the `main` branch. This branch (`MiniProject`) contains only the miniproject.

## Project Structure

```
mwit-iot-workshop/        (MiniProject branch)
  src/
    main.cpp              - firmware entry: boot, WiFi, OTA, web server
    webui.h               - inline HTML control panel served from flash
  include/
    credentials.example.h - template; copy to credentials.h (gitignored)
  lib/                    - shared libraries
    DHT22/                - DHT22 sensor wrapper with validation
    GG-Sheet/             - Google Sheets API client for ESP32
    LED/                  - LED control with blink patterns
    Log/                  - unified logger (Serial + telnet, boot buffer)
    MWIT-WiFi/            - WPA2-Enterprise WiFi manager
    OTA/                  - ArduinoOTA wrapper library
    BlynkClient/          - Blynk IoT client
    DiscordWebhook/       - Discord webhook client
  OTA/                    - bootstrap firmware + upload scripts (ota_upload.py, netmon.sh)
  partitions_ota.csv      - OTA-enabled partition table
  platformio.ini          - single project: esp32dev (USB) + esp32dev_ota (OTA)
```

## Planned Alarm Design

| Element | Role |
|---------|------|
| HC-SR04 ultrasonic | motion trigger |
| Status LEDs | armed / alarm indication |
| Relay | siren / alert output |
| Button | arm/disarm toggle + reset |
| Local web page | arm/disarm control + status |
| Discord webhook | intrusion alert push (when online) |
| Google Sheets | event log (when online) |

State machine: `DISARMED → ARMED → ALARM` — button toggles arm/disarm and resets an active alarm. All states function without network.

## Getting Started

### Hardware Requirements

- ESP32-DevKitC or compatible board (4 MB flash)
- HC-SR04 ultrasonic sensor, LEDs, push button, relay module
- USB cable for initial firmware flashing

### Software Requirements

- [PlatformIO](https://platformio.org/)
- Python 3 for the OTA upload script

### Configuration

Credentials load from `include/credentials.h` (gitignored):

```bash
cp include/credentials.example.h include/credentials.h
```

Fill in MWIT WPA2-Enterprise credentials (EAP identity, username, password).

### Building and Flashing

```bash
~/.platformio/penv/bin/platformio run -e esp32dev          # build
~/.platformio/penv/bin/platformio run -e esp32dev -t upload # first flash via USB
```

### OTA Updates (after first USB flash)

Once the ESP32 is on WiFi, set `upload_port` in `platformio.ini` (`[env:esp32dev_ota]`) to the device IP, then:

```bash
~/.platformio/penv/bin/platformio run -e esp32dev_ota
# or
python OTA/ota_upload.py .pio/build/esp32dev/firmware.bin <esp32-ip>
```

### Monitoring

```bash
nc <esp32-ip> 2323          # telnet log over network
OTA/netmon.sh <esp32-ip>    # same, with clean disconnect
```

## Shared Libraries

- **DHT22** — structured temperature/humidity reads with error handling
- **GG-Sheet** — HTTP client pushing data to Google Sheets via Apps Script Web App
- **LED** — non-blocking blink, pulse, and count patterns
- **Log** — dual-output logger (Serial + telnet) with 2 KB boot buffer
- **MWIT-WiFi** — WPA2-Enterprise connection setup for the MWIT-WiFi network
- **OTA** — ArduinoOTA wrapper with simplified begin/handle interface
- **BlynkClient** — Blynk IoT platform client
- **DiscordWebhook** — Discord webhook message client

## License

MIT License — see [LICENSE](LICENSE) for details.
