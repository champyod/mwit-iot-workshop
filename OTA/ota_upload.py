#!/usr/bin/env python3
"""
OTA firmware upload for ESP32
Usage:
  python ota_upload.py <firmware.bin> [IP] [port]
  python ota_upload.py firmware.bin 192.168.1.100
  python ota_upload.py firmware.bin activity6-ota.local 3232
"""
import sys, os, socket, struct, hashlib, time

def ota_upload(firmware_path, host, port=3232, password=""):
    if not os.path.exists(firmware_path):
        print(f"[ERR] Firmware not found: {firmware_path}")
        return False

    size = os.path.getsize(firmware_path)
    print(f"[OTA] Uploading {firmware_path} ({size} bytes) to {host}:{port}")

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(30)
        # Resolve mDNS (.local)
        if host.endswith(".local"):
            import socket as _s
            host = _s.gethostbyname(host)
        sock.connect((host, port))
    except Exception as e:
        print(f"[ERR] Connection failed: {e}")
        return False

    # ArduinoOTA protocol handshake
    auth = password.encode() + b"\x00"
    cmd = struct.pack("<II", 0, size) + auth
    sock.send(cmd)

    # Send firmware in chunks
    with open(firmware_path, "rb") as f:
        sent = 0
        while True:
            chunk = f.read(4096)
            if not chunk:
                break
            sock.send(chunk)
            sent += len(chunk)
            pct = int(sent / size * 100)
            bar = "#" * (pct // 5) + "-" * (20 - pct // 5)
            print(f"\r[OTA] [{bar}] {pct}%", end="", flush=True)

    print(f"\n[OTA] Done — {sent} bytes sent")
    sock.close()
    return True

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <firmware.bin> [host=activity6-ota.local] [port=3232]")
        sys.exit(1)

    fw = sys.argv[1]
    host = sys.argv[2] if len(sys.argv) > 2 else "activity6-ota.local"
    port = int(sys.argv[3]) if len(sys.argv) > 3 else 3232
    sys.exit(0 if ota_upload(fw, host, port) else 1)
