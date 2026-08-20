#!/usr/bin/env bash
# netmon.sh — auto-reconnecting netcat monitor for ESP32 telnet log (port 2323).
# Survives OTA reboots: firmware sends FIN before reboot -> nc exits cleanly ->
# loop echoes status, waits, reconnects. Use: ./netmon.sh <ip> [port]
set -u
HOST="${1:?usage: netmon.sh <ip> [port]}"
PORT="${2:-2323}"

echo "[netmon] monitoring ${HOST}:${PORT} — Ctrl+C to stop"
while true; do
    nc "${HOST}" "${PORT}"
    echo "[netmon] connection lost — retrying in 3s"
    sleep 3
done