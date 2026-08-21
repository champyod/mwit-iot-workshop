#pragma once

// Inline control-panel page served from flash. Kept as a raw literal so the
// skeleton needs no SPIFFS image or external asset step.
const char WEBUI_PAGE[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>MiniProject Alarm</title>
<style>
  :root { color-scheme: dark; }
  body {
    margin: 0; min-height: 100vh; display: flex; align-items: center;
    justify-content: center; background: #10141c; color: #e8ecf4;
    font-family: system-ui, sans-serif;
  }
  .card {
    width: min(420px, 92vw); background: #1a2130; border-radius: 14px;
    padding: 24px; box-shadow: 0 8px 30px rgba(0,0,0,.45);
  }
  h1 { margin: 0 0 4px; font-size: 20px; }
  .sub { color: #8b96ab; font-size: 13px; margin-bottom: 18px; }
  dl { display: grid; grid-template-columns: 1fr auto; gap: 10px 16px; margin: 0; }
  dt { color: #8b96ab; font-size: 13px; align-self: center; }
  dd { margin: 0; font-family: ui-monospace, monospace; font-size: 14px; text-align: right; }
  .ok { color: #5dd39e; } .bad { color: #ef6a6a; } .warn { color: #e8b64c; }
  button {
    margin-top: 18px; width: 100%; padding: 10px 0; border-radius: 8px;
    border: 0; background: #2a3550; color: #e8ecf4; font-size: 14px;
    cursor: pointer; font-family: inherit;
  }
  button:hover { background: #34426a; }
</style>
</head>
<body>
<div class="card">
  <h1>MiniProject Alarm</h1>
  <div class="sub">system status &mdash; refreshes every 1 s</div>
  <dl>
    <dt>WiFi</dt><dd id="wifi">&mdash;</dd>
    <dt>IP</dt><dd id="ip">&mdash;</dd>
    <dt>RSSI</dt><dd id="rssi">&mdash;</dd>
    <dt>Free heap</dt><dd id="heap">&mdash;</dd>
    <dt>Uptime</dt><dd id="uptime">&mdash;</dd>
    <dt>Engine</dt><dd id="engine">&mdash;</dd>
    <dt>Tier</dt><dd id="tier">&mdash;</dd>
    <dt>Nearest</dt><dd id="nearest">&mdash;</dd>
  </dl>
  <button id="toggleBtn" onclick="toggleEngine()">PAUSE</button>
</div>
<script>
function fmtUptime(ms) {
  const s = Math.floor(ms / 1000);
  const d = Math.floor(s / 86400), h = Math.floor(s % 86400 / 3600),
        m = Math.floor(s % 3600 / 60), sec = s % 60;
  return d ? `${d}d ${h}h ${m}m` : `${h}h ${m}m ${sec}s`;
}
async function poll() {
  try {
    const r = await fetch('/api/status');
    const j = await r.json();
    const wifiEl = document.getElementById('wifi');
    wifiEl.textContent = j.wifi_connected ? 'connected' : 'offline';
    wifiEl.className = j.wifi_connected ? 'ok' : 'bad';
    document.getElementById('ip').textContent = j.ip;
    document.getElementById('rssi').textContent = j.rssi_dbm + ' dBm';
    document.getElementById('heap').textContent =
      (j.free_heap / 1024).toFixed(0) + ' KB';
    document.getElementById('uptime').textContent = fmtUptime(j.uptime_ms);
    const engEl = document.getElementById('engine');
    engEl.textContent = j.running ? 'RUNNING' : 'PAUSED';
    engEl.className = j.running ? 'ok' : '';
    const tierEl = document.getElementById('tier');
    tierEl.textContent = j.tier;
    tierEl.className =
      j.tier === 'DANGER' ? 'bad' : (j.tier === 'WARN' ? 'warn' : 'ok');
    document.getElementById('nearest').textContent =
      j.nearest_cm >= 0 ? j.nearest_cm.toFixed(1) + ' cm' : '\u2014';
    document.getElementById('toggleBtn').textContent = j.running ? 'PAUSE' : 'RUN';
  } catch (e) { /* device busy or link down; next tick retries */ }
}
async function toggleEngine() {
  try {
    await fetch('/api/toggle', { method: 'POST' });
    poll();
  } catch (e) { /* retried on next poll tick */ }
}
poll();
setInterval(poll, 1000);
</script>
</body>
</html>)rawliteral";
