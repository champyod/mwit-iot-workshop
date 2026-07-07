# MWIT IoT Workshop

Progressive ESP32 firmware collection covering WiFi connectivity, sensor integration, web server control, and cloud data logging with OTA updates. Built for the MWIT (Mahidol Wittayanusorn School) IoT curriculum using PlatformIO and the Arduino framework.

## Project Structure

```
mwit-iot-workshop/
  activity1/         - Basic DHT22 sensor reading via Serial
  activity2/         - WiFi scanning with WPA2-Enterprise authentication
  activity3/         - Web server serving a static HTML page over WiFi
  activity4/         - DHT22 sensor dashboard with real-time web UI + heat index
  activity5/         - 3-LED web control with on/off toggle (SPIFFS variant)
  activity5-simple/  - 3-LED web control (inline HTML, no SPIFFS)
  activity5-slider/  - 3-LED PWM brightness control with slider UI
  activity6/         - DHT22 data logger with Google Sheets + OTA updates
  OTA/               - Bootstrap firmware and upload scripts for OTA workflow
  lib/
    DHT22/           - DHT22 sensor wrapper with validation
    GG-Sheet/        - Google Sheets API client for ESP32
    LED/             - LED control with blink patterns
    Log/             - Unified logger (Serial + telnet with boot buffer)
    MWIT-WiFi/       - WPA2-Enterprise WiFi manager
    OTA/             - ArduinoOTA wrapper library
```

## Activities

| Activity | Topic | Highlights |
|----------|-------|------------|
| 1 | DHT22 Basics | Serial output, temperature/humidity reading |
| 2 | WiFi Scanning | WPA2-Enterprise auth, SSID scan, disconnect diagnosis |
| 3 | Web Server | Static HTML page served from ESP32 |
| 4 | Sensor Dashboard | Real-time web UI with DHT22 + heat index, JSON API |
| 5 | LED Web Control | 3-LED on/off toggling via REST API |
| 5-simple | LED Control (basic) | Inline HTML version, no filesystem |
| 5-slider | PWM LED Control | Brightness slider, SPIFFS-served static assets |
| 6 | Cloud Logger | DHT22 -> Google Sheets, OTA updates, NTP sync |

## Getting Started

### Hardware Requirements

- ESP32-DevKitC or compatible board (4 MB flash)
- DHT22 temperature and humidity sensor
- LEDs with current-limiting resistors (activities 5+)
- USB cable for initial firmware flashing

### Software Requirements

- [PlatformIO](https://platformio.org/) (`pip install platformio`)
- For WPA2-Enterprise networks: network credentials configured in source code
- Python 3 for the OTA upload script

### Building and Flashing

Each activity is a standalone PlatformIO project. Build and upload from the activity directory:

```bash
cd activity1
platformio run -e esp32dev -t upload
platformio device monitor
```

For activities 5-slider and 5, upload the SPIFFS filesystem first:

```bash
cd activity5-slider
platformio run -e esp32dev -t uploadfs
platformio run -e esp32dev -t upload
```

### OTA Workflow (Activity 6 and later)

First flash must be via USB. After the ESP32 connects to WiFi, subsequent updates use OTA.

**First flash (USB):**

```bash
cd activity6
platformio run -e esp32dev -t upload
```

**OTA update:**

```bash
cd activity6
platformio run -e esp32dev
python ../OTA/ota_upload.py .pio/build/esp32dev/firmware.bin <esp32-ip>
```

See `OTA/README.md` for detailed OTA instructions.

## Shared Libraries

The `lib/` directory contains reusable components shared across activities:

- **DHT22** - Wraps DHT sensor library with structured read results and error handling
- **GG-Sheet** - HTTP client for pushing data to Google Sheets via Apps Script Web App
- **LED** - Non-blocking LED control with blink, pulse, and count patterns
- **Log** - Dual-output logger writing to both Serial (USB) and telnet clients, with a 2 KB boot buffer for messages before a telnet client connects
- **MWIT-WiFi** - Encapsulates WPA2-Enterprise connection setup for MWIT-WiFi network
- **OTA** - ArduinoOTA wrapper with simplified begin/handle interface

## Configuration

Credentials are loaded from `include/credentials.h` (gitignored). Copy the template and fill in your values:

```bash
cp include/credentials.example.h include/credentials.h
```

Edit `include/credentials.h` with your network credentials:

- WiFi SSID and WPA2-Enterprise credentials (EAP identity, username, password)
- Google Sheets Web App URL (activity 6 only)
- OTA hostname and auth password (activity 6 only)

The defines in `credentials.h` use `#ifndef` guards, so values can also be overridden via PlatformIO `build_flags` in each activity's `platformio.ini`:

```ini
build_flags =
    -D WIFI_SSID='"your-ssid"'
    -D WIFI_PASSWORD='"your-password"'
```

## License

MIT License - see [LICENSE](LICENSE) for details.
