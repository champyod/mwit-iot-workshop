#!/usr/bin/env python3
"""
OTA firmware upload for ESP32
Usage:
  python ota_upload.py <firmware.bin> [IP] [port]
  python ota_upload.py firmware.bin 192.168.1.100
  python ota_upload.py firmware.bin activity6-ota.local 3232
"""
import sys, os, socket, struct, hashlib, time

def load_env():
    """Read .env from same directory as this script."""
    env_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), ".env")
    env = {}
    if os.path.exists(env_path):
        with open(env_path) as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith("#") or "=" not in line:
                    continue
                k, _, v = line.partition("=")
                env[k.strip()] = v.strip()
    return env

def ota_upload(firmware_path, host, port=3232, password=""):
    if not os.path.exists(firmware_path):
        print(f"[ERR] Firmware not found: {firmware_path}")
        return False

    size = os.path.getsize(firmware_path)
    print(f"[OTA] Uploading {firmware_path} ({size} bytes) to {host}:{port}")

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(60)
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
    env = load_env()

    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <firmware.bin> [host] [port]")
        print(f"  Default host from .env: {env.get('OTA_HOST', '(not set)')}")
        sys.exit(1)

    fw = sys.argv[1]
    host = sys.argv[2] if len(sys.argv) > 2 else env.get("OTA_HOST", "activity6-ota.local")
    port = int(sys.argv[3] if len(sys.argv) > 3 else env.get("OTA_PORT", "3232"))
    sys.exit(0 if ota_upload(fw, host, port) else 1)
