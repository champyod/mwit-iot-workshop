#pragma once

const char WEBUI_PAGE[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>MiniProject Alarm</title>
<style>
:root{--bg:#0e131e;--card:#171e2e;--card2:#1e2942;--line:#2a3652;--text:#e8ecf4;--muted:#8b96ab;--ok:#5dd39e;--warn:#e8b64c;--bad:#ef6a6a;--radius:16px;--pad:20px}
*{box-sizing:border-box}html,body{margin:0;min-height:100%;background:var(--bg);color:var(--text);font-family:system-ui,-apple-system,Segoe UI,Roboto,sans-serif}
.page{max-width:960px;margin:0 auto;padding:20px 16px 32px}
header{display:flex;flex-wrap:wrap;gap:12px;align-items:center;justify-content:space-between;margin-bottom:16px}
.brand h1{margin:0;font-size:20px;letter-spacing:.2px}
.brand p{margin:4px 0 0;color:var(--muted);font-size:13px}
.pills{display:flex;gap:8px;flex-wrap:wrap}
.pill{padding:6px 12px;border-radius:999px;font-size:12px;font-weight:600;letter-spacing:.3px;border:1px solid var(--line);background:var(--card2);color:var(--muted)}
.pill.ok{color:var(--ok);border-color:rgba(93,211,158,.3)}
.pill.warn{color:var(--warn);border-color:rgba(232,182,76,.3)}
.pill.bad{color:var(--bad);border-color:rgba(239,106,106,.3)}
.grid{display:grid;grid-template-columns:1.2fr .8fr;gap:14px}
@media(max-width:720px){.grid{grid-template-columns:1fr}}
.card{background:var(--card);border:1px solid var(--line);border-radius:var(--radius);padding:var(--pad);box-shadow:0 8px 30px rgba(0,0,0,.35)}
.card h2{margin:0 0 12px;font-size:14px;letter-spacing:.4px;text-transform:uppercase;color:var(--muted)}
.kvs{display:grid;grid-template-columns:1fr auto;gap:10px 16px}
.kvs dt{color:var(--muted);font-size:13px}
.kvs dd{margin:0;font-family:ui-monospace,monospace;font-size:13px;text-align:right}
.sensors table{width:100%;border-collapse:collapse}
.sensors th{color:var(--muted);font-size:11px;text-transform:uppercase;letter-spacing:.4px;text-align:left;padding:8px 6px;border-bottom:1px solid var(--line)}
.sensors td{padding:10px 6px;border-bottom:1px solid rgba(42,54,82,.6);font-family:ui-monospace,monospace;font-size:13px}
.sensors td:first-child{font-family:system-ui,sans-serif;font-weight:600}
.badge{padding:3px 8px;border-radius:999px;font-size:11px;font-weight:700}
.badge.safe{background:rgba(93,211,158,.15);color:var(--ok)}
.badge.warn{background:rgba(232,182,76,.15);color:var(--warn)}
.badge.danger{background:rgba(239,106,106,.15);color:var(--bad)}
.inputs{display:grid;grid-template-columns:1fr 1fr;gap:10px}
@media(max-width:480px){.inputs{grid-template-columns:1fr}}
.field label{display:block;color:var(--muted);font-size:11px;letter-spacing:.3px;text-transform:uppercase;margin-bottom:6px}
.field input{width:100%;background:var(--card2);border:1px solid var(--line);color:var(--text);border-radius:10px;padding:10px 12px;font-size:14px;outline:none}
.field input:focus{border-color:#3a4a72;box-shadow:0 0 0 3px rgba(58,74,114,.3)}
.actions{display:flex;gap:8px;margin-top:12px;flex-wrap:wrap}
.btn{appearance:none;border:0;border-radius:10px;padding:10px 14px;font-size:13px;font-weight:600;cursor:pointer}
.btn-primary{background:#2f3f62;color:var(--text)}
.btn-primary:hover{background:#34466e}
.btn-ghost{background:transparent;color:var(--muted);border:1px solid var(--line)}
.btn-ghost:hover{color:var(--text);border-color:#3a4a72}
.btn-danger{background:rgba(239,106,106,.12);color:var(--bad);border:1px solid rgba(239,106,106,.25)}
.btn-danger:hover{background:rgba(239,106,106,.18)}
.mono{font-family:ui-monospace,monospace}
.muted{color:var(--muted);font-size:12px}
details.debug{margin-top:14px;background:var(--card);border:1px solid var(--line);border-radius:var(--radius);padding:0}
details.debug summary{list-style:none;cursor:pointer;padding:14px var(--pad);color:var(--muted);font-size:13px;display:flex;align-items:center;justify-content:space-between}
details.debug summary::-webkit-details-marker{display:none}
details.debug[open] summary{border-bottom:1px solid var(--line)}
.debug-body{padding:14px var(--pad)}
.stack{display:flex;flex-direction:column;gap:14px}
@media(max-width:640px){.sensors table thead{display:none}.sensors table tr{display:grid;grid-template-columns:1fr 1fr;gap:4px;padding:8px 0}.sensors table td{border:0;padding:4px 6px}.sensors table td::before{content:attr(data-l);color:var(--muted);font-size:11px;text-transform:uppercase;letter-spacing:.3px;display:block}}
</style>
</head>
<body>
<div class="page">
<header>
<div class="brand"><h1>MiniProject Alarm</h1><p>Live distance &amp; zone control &mdash; updates 1 s</p></div>
<div class="pills"><span id="pillEngine" class="pill">PAUSED</span><span id="pillTier" class="pill">SAFE</span><span id="pillNearest" class="pill">—</span></div>
</header>
<div class="grid">
<div class="card">
<h2>System</h2>
<div class="kvs"><dt>Engine</dt><dd id="engine">—</dd><dt>Tier</dt><dd id="tier">—</dd><dt>Nearest</dt><dd id="nearest">—</dd></div>
<div class="actions"><button id="toggleBtn" class="btn btn-primary" onclick="toggleEngine()">RUN</button><button class="btn btn-ghost" onclick="doReset()">Reset defaults</button></div>
<p class="muted" style="margin:10px 0 0">Short press physical button = toggle. Hold 3 s = reset thresholds &amp; calibration.</p>
</div>
<div class="card">
<h2>Range config <span class="muted" style="font-weight:400;text-transform:none;letter-spacing:0">live, resets on reboot</span></h2>
<div class="inputs">
<div class="field"><label>Danger ≤ cm</label><input id="dangerIn" type="number" step="1" min="5" max="400" value="50"></div>
<div class="field"><label>Warn ≤ cm</label><input id="warnIn" type="number" step="1" min="5" max="450" value="100"></div>
</div>
<div class="actions"><button class="btn btn-primary" onclick="applyThresholds()">Apply thresholds</button><span id="thMsg" class="muted"></span></div>
</div>
</div>
<div class="card sensors" style="margin-top:14px">
<h2>Ultrasonic sensors</h2>
<table><thead><tr><th>#</th><th>Raw</th><th>Corrected</th><th>Offset cm</th><th>Scale ×</th><th>State</th></tr></thead><tbody id="sensorBody"></tbody></table>
<div class="actions"><button class="btn btn-primary" onclick="applyCalibration()">Apply calibration</button><span id="calMsg" class="muted"></span></div>
<p class="muted" id="calHint" style="margin:8px 0 0">Offset −50…+50 cm, scale 0.5…2.0. Corrected = (raw + offset) × scale.</p>
</div>
<details class="debug"><summary>Debug details <span class="muted">WiFi · heap · uptime</span></summary><div class="debug-body"><div class="kvs"><dt>WiFi</dt><dd id="wifi">—</dd><dt>IP</dt><dd id="ip">—</dd><dt>RSSI</dt><dd id="rssi">—</dd><dt>Free heap</dt><dd id="heap">—</dd><dt>Uptime</dt><dd id="uptime">—</dd></div></div></details>
</div>
<script>
function fmtUptime(ms){const s=Math.floor(ms/1000),d=Math.floor(s/86400),h=Math.floor(s%86400/3600),m=Math.floor(s%3600/60),sec=s%60;return d?`${d}d ${h}h ${m}m`:`${h}h ${m}m ${sec}s`}
function tierClass(t){return t==='DANGER'?'bad':t==='WARN'?'warn':'ok'}
function badgeFor(cm,t){if(cm<0) return '<span class="badge safe">no echo</span>';if(t==='DANGER') return '<span class="badge danger">DANGER</span>';if(t==='WARN') return '<span class="badge warn">WARN</span>';return '<span class="badge safe">SAFE</span>'}
let lastSensors=[];
async function poll(){
 try{
  const r=await fetch('/api/status');const j=await r.json();
  const eng=j.running?'RUNNING':'PAUSED';
  document.getElementById('engine').textContent=eng;
  document.getElementById('tier').textContent=j.tier;
  document.getElementById('tier').className=j.tier==='DANGER'?'bad':j.tier==='WARN'?'warn':'ok';
  document.getElementById('nearest').textContent=j.nearest_cm>=0?j.nearest_cm.toFixed(1)+' cm':'—';
  const pe=document.getElementById('pillEngine');pe.textContent=eng;pe.className='pill '+(j.running?'ok':'');
  const pt=document.getElementById('pillTier');pt.textContent=j.tier;pt.className='pill '+tierClass(j.tier);
  const pn=document.getElementById('pillNearest');pn.textContent=j.nearest_cm>=0?j.nearest_cm.toFixed(1)+' cm':'—';
  document.getElementById('toggleBtn').textContent=j.running?'PAUSE':'RUN';
  const wifiEl=document.getElementById('wifi');wifiEl.textContent=j.wifi_connected?'connected':'offline';wifiEl.className=j.wifi_connected?'ok':'bad';
  document.getElementById('ip').textContent=j.ip;
  document.getElementById('rssi').textContent=j.rssi_dbm+' dBm';
  document.getElementById('heap').textContent=(j.free_heap/1024).toFixed(0)+' KB';
  document.getElementById('uptime').textContent=fmtUptime(j.uptime_ms);
  if(document.activeElement!==document.getElementById('dangerIn') && document.activeElement!==document.getElementById('warnIn')){
    document.getElementById('dangerIn').value=j.danger_cm;document.getElementById('warnIn').value=j.warn_cm;
  }
  const body=document.getElementById('sensorBody');body.innerHTML='';
  lastSensors=j.sensors||[];
  lastSensors.forEach((s,i)=>{
    const tr=document.createElement('tr');
    const rawTxt=s.raw>=0?s.raw.toFixed(1)+' cm':'—';
    const cmTxt=s.cm>=0?s.cm.toFixed(1)+' cm':'—';
    tr.innerHTML=`<td data-l="#">${i+1}</td><td data-l="Raw">${rawTxt}</td><td data-l="Corrected">${cmTxt}</td><td data-l="Offset"><input id="off${i}" type="number" step="0.1" value="${s.offset}" style="width:90px;background:var(--card2);border:1px solid var(--line);color:var(--text);border-radius:8px;padding:6px 8px"></td><td data-l="Scale"><input id="scale${i}" type="number" step="0.05" value="${s.scale}" style="width:80px;background:var(--card2);border:1px solid var(--line);color:var(--text);border-radius:8px;padding:6px 8px"></td><td data-l="State">${s.cm>=0?badgeFor(s.cm,j.tier):'<span class="muted">no echo</span>'}</td>`;
    body.appendChild(tr);
  });
 }catch(e){}
}
async function toggleEngine(){try{await fetch('/api/toggle',{method:'POST'});poll()}catch(e){}}
async function applyThresholds(){
 const d=parseFloat(document.getElementById('dangerIn').value),w=parseFloat(document.getElementById('warnIn').value);
 try{await fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({danger_cm:d,warn_cm:w})});document.getElementById('thMsg').textContent='Applied';setTimeout(()=>document.getElementById('thMsg').textContent='',1500);poll()}catch(e){document.getElementById('thMsg').textContent='Failed'}
}
async function applyCalibration(){
 if(!lastSensors.length) return;const sensors=lastSensors.map((_,i)=>({offset:parseFloat(document.getElementById('off'+i).value),scale:parseFloat(document.getElementById('scale'+i).value)}));
 try{await fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({sensors})});document.getElementById('calMsg').textContent='Applied';setTimeout(()=>document.getElementById('calMsg').textContent='',1500);poll()}catch(e){document.getElementById('calMsg').textContent='Failed'}
}
async function doReset(){try{await fetch('/api/reset',{method:'POST'});poll()}catch(e){}}
poll();setInterval(poll,1000);
</script>
</body>
</html>)rawliteral";
