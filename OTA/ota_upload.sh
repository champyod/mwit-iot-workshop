#!/bin/bash
# ESP32 OTA upload helper
# Usage: ./ota_upload.sh <firmware.bin> [host] [port]
#
# Build + upload example:
#   pio run -e esp32dev              # builds firmware
#   ./ota_upload.sh .pio/build/esp32dev/firmware.bin 192.168.1.100

FW="${1:-.pio/build/esp32dev/firmware.bin}"
HOST="${2:-activity6-ota.local}"
PORT="${3:-3232}"

if [ ! -f "$FW" ]; then
    echo "ERR: $FW not found"
    echo "Usage: $0 <firmware.bin> [host] [port]"
    exit 1
fi

SIZE=$(stat -c%s "$FW")
echo "→ Uploading $FW ($SIZE bytes)"
echo "→ Target: $HOST:$PORT"

python3 -c "
import socket, struct
s = socket.socket()
s.settimeout(60)
s.connect(('$HOST', $PORT))
# ArduinoOTA handshake: command=0 (flash), size, password (none)
s.send(struct.pack('<II', 0, $SIZE) + b'\x00')
with open('$FW', 'rb') as f:
    while True:
        c = f.read(4096)
        if not c: break
        s.send(c)
s.close()
print('✓ OTA upload complete — device is rebooting')
" && echo "Done" || echo "FAILED"
