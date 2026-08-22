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
header{margin-bottom:16px}
.brand h1{margin:0;font-size:20px;letter-spacing:.2px}
.grid{display:grid;grid-template-columns:1.2fr .8fr;gap:14px}
@media(max-width:720px){.grid{grid-template-columns:1fr}}
.card{background:var(--card);border:1px solid var(--line);border-radius:var(--radius);padding:var(--pad);box-shadow:0 8px 30px rgba(0,0,0,.35)}
.card h2{margin:0 0 12px;font-size:14px;letter-spacing:.4px;text-transform:uppercase;color:var(--muted)}
.sysTop{display:flex;align-items:center;justify-content:space-between;gap:12px;flex-wrap:wrap;margin-bottom:6px}
.bigNum{font-size:36px;font-weight:700;font-family:ui-monospace,monospace;line-height:1.05}
.bigNum .unit{font-size:14px;color:var(--muted);font-weight:600;margin-left:4px}
.tierBadge{padding:7px 14px;border-radius:999px;font-size:15px;font-weight:800;letter-spacing:.5px}
.tierBadge.ok{background:rgba(93,211,158,.15);color:var(--ok)}
.tierBadge.warn{background:rgba(232,182,76,.15);color:var(--warn)}
.tierBadge.bad{background:rgba(239,106,106,.18);color:var(--bad)}
.radar{position:relative;margin-top:10px;background:var(--card2);border:1px solid var(--line);border-radius:12px;padding:4px 6px 0}
.radar svg{width:100%;height:auto;display:block}
.noSig{position:absolute;inset:0;display:flex;align-items:center;justify-content:center;color:var(--muted);font-size:12px;font-weight:700;letter-spacing:1px}
.kvs{display:grid;grid-template-columns:1fr auto;gap:10px 16px}
.kvs dt{color:var(--muted);font-size:13px}
.kvs dd{margin:0;font-family:ui-monospace,monospace;font-size:13px;text-align:right}
.sensors table{width:100%;border-collapse:collapse}
.sensors th{color:var(--muted);font-size:11px;text-transform:uppercase;letter-spacing:.4px;text-align:left;padding:8px 6px;border-bottom:1px solid var(--line)}
.sensors td{padding:10px 6px;border-bottom:1px solid rgba(42,54,82,.6);font-family:ui-monospace,monospace;font-size:13px}
.sensors td:first-child{font-family:system-ui,sans-serif;font-weight:600}
tr.rowOff{filter:grayscale(1);opacity:.5}
.badge{padding:3px 8px;border-radius:999px;font-size:11px;font-weight:700}
.badge.safe{background:rgba(93,211,158,.15);color:var(--ok)}
.badge.warn{background:rgba(232,182,76,.15);color:var(--warn)}
.badge.danger{background:rgba(239,106,106,.15);color:var(--bad)}
.inputs{display:grid;grid-template-columns:1fr 1fr;gap:10px}
@media(max-width:480px){.inputs{grid-template-columns:1fr}}
.field label{display:block;color:var(--muted);font-size:11px;letter-spacing:.3px;text-transform:uppercase;margin-bottom:6px}
.field input{width:100%;background:var(--card2);border:1px solid var(--line);color:var(--text);border-radius:10px;padding:10px 12px;font-size:14px;outline:none}
.field input:focus{border-color:#3a4a72;box-shadow:0 0 0 3px rgba(58,74,114,.3)}
.actions{display:flex;gap:8px;margin-top:12px;flex-wrap:wrap;align-items:center}
.btn{appearance:none;border:0;border-radius:10px;padding:10px 14px;font-size:13px;font-weight:600;cursor:pointer}
.btn-primary{background:#2f3f62;color:var(--text)}
.btn-primary:hover{background:#34466e}
.btn-ghost{background:transparent;color:var(--muted);border:1px solid var(--line)}
.btn-ghost:hover{color:var(--text);border-color:#3a4a72}
.btn-start{background:#1d9d5f;color:#ffffff}
.btn-start:hover{background:#21b06c}
.btn-stop{background:#cf4444;color:#ffffff}
.btn-stop:hover{background:#dd5555}
.btn-reset{background:transparent;color:var(--bad);border:1px solid rgba(239,106,106,.45)}
.btn-reset:hover{background:rgba(239,106,106,.08);border-color:rgba(239,106,106,.7)}
.pop{animation:popin .35s ease-out}
@keyframes popin{from{opacity:0}to{opacity:1}}
.btn[disabled]{opacity:.55;cursor:not-allowed}
.spin{display:inline-block;width:11px;height:11px;border:2px solid currentColor;border-top-color:transparent;border-radius:50%;animation:rot .7s linear infinite;vertical-align:-1px}
@keyframes rot{to{transform:rotate(360deg)}}
.muted{color:var(--muted);font-size:12px}
details.debug{margin-top:14px;background:var(--card);border:1px solid var(--line);border-radius:var(--radius);padding:0}
details.debug summary{list-style:none;cursor:pointer;padding:14px var(--pad);color:var(--muted);font-size:13px;display:flex;align-items:center;justify-content:space-between}
details.debug summary::-webkit-details-marker{display:none}
details.debug[open] summary{border-bottom:1px solid var(--line)}
.debug-body{padding:14px var(--pad)}
@media(max-width:640px){.sensors table thead{display:none}.sensors table tr{display:grid;grid-template-columns:1fr 1fr;gap:4px;padding:8px 0}.sensors table td{border:0;padding:4px 6px}.sensors table td::before{content:attr(data-l);color:var(--muted);font-size:11px;text-transform:uppercase;letter-spacing:.3px;display:block}}
</style>
</head>
<body>
<div class="page">
<header><div class="brand"><h1>MiniProject Alarm</h1></div></header>
<div class="grid">
<div class="card">
<h2>System</h2>
<div class="sysTop">
<div><div class="bigNum"><span id="nearest">—</span><span class="unit">cm nearest</span></div></div>
<span id="tierBadge" class="tierBadge ok">SAFE</span>
<button id="toggleBtn" class="btn btn-start" onclick="toggleEngine()" disabled>START</button>
</div>
<div class="radar"><svg id="radarSvg" viewBox="0 0 320 118" aria-hidden="true"></svg><div id="noSig" class="noSig" style="display:none">NO SIGNAL</div><div id="syncing" class="noSig" style="display:none;z-index:2;background:rgba(14,19,30,.55)"><span class="spin"></span>&nbsp;SYNCING</div></div>
<div class="actions"><button class="btn btn-reset" id="resetBtn" onclick="doReset()" disabled>Reset defaults</button><span class="muted">Link delay <b id="latency" class="mono">—</b></span></div>
<p class="muted" style="margin:10px 0 0">Short press physical button = start/stop. Hold 3 s = reset thresholds &amp; calibration.</p>
</div>
<div class="card">
<h2>Range config <span class="muted" style="font-weight:400;text-transform:none;letter-spacing:0">live, resets on reboot</span></h2>
<div class="inputs">
<div class="field"><label>Danger ≤ cm</label><input id="dangerIn" type="number" step="1" min="5" max="400" placeholder="—"></div>
<div class="field"><label>Warn ≤ cm</label><input id="warnIn" type="number" step="1" min="5" max="450" placeholder="—"></div>
</div>
<div class="actions"><button class="btn btn-primary" id="thBtn" onclick="applyThresholds()" disabled>Apply thresholds</button><span id="thMsg" class="muted"></span></div>
<h2 style="margin-top:16px">Timing</h2>
<div class="inputs">
<div class="field"><label>Sample every ms (10+)</label><input id="intervalIn" type="number" step="10" min="10" max="60000" placeholder="—"></div>
<div class="field"><label>Echo window ms (≥45 for RCW-0001)</label><input id="timeoutIn" type="number" step="5" min="10" max="200" placeholder="—"></div>
</div>
<div class="actions"><button class="btn btn-primary" id="timeBtn" onclick="applyTiming()" disabled>Apply timing</button><span id="timeMsg" class="muted"></span></div>
<p class="muted" style="margin:8px 0 0">Interval = how often a detection cycle repeats. Echo window = max wait for the return pulse; below ~45 ms this module reports FAILED.</p>
</div>
</div>
<div class="card sensors" style="margin-top:14px">
<h2>Ultrasonic sensors</h2>
<table><thead><tr><th>#</th><th>Raw</th><th>Corrected</th><th>Offset cm</th><th>Scale ×</th><th>Delay ms</th><th>State</th><th>Pwr</th></tr></thead><tbody id="sensorBody"></tbody></table>
<div class="actions"><button class="btn btn-primary" id="calBtn" onclick="applyCalibration()" disabled>Apply calibration</button><span id="calMsg" class="muted"></span></div>
<p class="muted" id="calHint" style="margin:8px 0 0">Offset −50…+50 cm, scale 0.5…2.0. Corrected = (raw + offset) × scale. Grayed row = sensor off.</p>
</div>
<details class="debug"><summary>Debug details <span class="muted">WiFi · heap · uptime</span></summary><div class="debug-body"><div class="kvs"><dt>WiFi</dt><dd id="wifi">—</dd><dt>IP</dt><dd id="ip">—</dd><dt>RSSI</dt><dd id="rssi">—</dd><dt>Free heap</dt><dd id="heap">—</dd><dt>Uptime</dt><dd id="uptime">—</dd></div></div></details>
</div>
<script>
const AXIS_Y=100,X0=16,XW=288,BEAM_H=66,LOGMAX=Math.log(450);
const RINGS=[25,50,100,200,450];
const ringX=d=>X0+Math.log(d)/LOGMAX*XW;
function radarColor(t){return t==='DANGER'?'#ef6a6a':t==='WARN'?'#e8b64c':'#5dd39e'}
function buildRadar(){
 let s=`<line x1="${X0}" y1="${AXIS_Y}" x2="${X0+XW}" y2="${AXIS_Y}" stroke="#2a3652" stroke-width="1.5"/>`;
 RINGS.forEach((d,i)=>{
  const x=ringX(d),h=12+(d/450)*54;
  s+=`<line x1="${x}" y1="${AXIS_Y}" x2="${x}" y2="${AXIS_Y-h}" stroke="#2a3652" stroke-width="${i===RINGS.length-1?3:2.2}" stroke-linecap="round" opacity="${i===RINGS.length-1?.95:.6}"/>`;
  s+=`<text x="${x}" y="113" font-size="9" fill="#8b96ab" text-anchor="middle" font-family="ui-monospace,monospace">${d}</text>`;
 });
 s+=`<g id="beamG" style="display:none"><g id="beamIn"><line x1="${X0}" y1="${AXIS_Y}" x2="${X0}" y2="${AXIS_Y-BEAM_H-14}" stroke-width="7" stroke-linecap="round" opacity=".18"/><line x1="${X0}" y1="${AXIS_Y}" x2="${X0}" y2="${AXIS_Y-BEAM_H-14}" stroke-width="2" stroke-linecap="round"/></g></g>`;
 s+=`<line id="sweep" x1="${X0}" y1="${AXIS_Y-BEAM_H-14}" x2="${X0}" y2="${AXIS_Y}" stroke="rgba(232,236,244,.3)" stroke-width="2" stroke-linecap="round"/>`;
 document.getElementById('radarSvg').innerHTML=s;
}
let radarRun=false,radarCm=-1,tierCol='#5dd39e',curSample=100,sweepT0=performance.now(),lastPop=-1e9;
let awaiting=false;
function showSync(){awaiting=true;document.getElementById('syncing').style.display='flex'}
function hideSync(){if(!awaiting)return;awaiting=false;document.getElementById('syncing').style.display='none'}
function tick(now){
 const sw=document.getElementById('sweep'),g=document.getElementById('beamG'),inn=document.getElementById('beamIn');
 const live=radarRun&&radarCm>=0;
 document.getElementById('noSig').style.display=live?'none':'flex';
 if(!live){sw.style.display='none';g.style.display='none';requestAnimationFrame(tick);return}
 const T=Math.min(Math.max(curSample,800),4000);
 const ph=((now-sweepT0)%T)/T,sx=X0+ph*XW;
 sw.style.display='';
 sw.setAttribute('transform','translate('+(sx-X0).toFixed(1)+',0)');
 const tx=ringX(radarCm);
 g.style.display='';
 g.setAttribute('transform','translate('+(tx-X0).toFixed(1)+',0)');
 inn.querySelectorAll('line').forEach(l=>l.setAttribute('stroke',tierCol));
 if(sx>=tx&&now-lastPop>T*0.55){
  lastPop=now;
  inn.classList.remove('pop');void inn.getBoundingClientRect();inn.classList.add('pop');
 }
 inn.style.opacity=Math.max(0,1-(now-lastPop)/(T*0.85)).toFixed(2);
 requestAnimationFrame(tick);
}
function updateRadar(cm,tier){
 radarCm=(radarRun&&cm>=0&&cm<=450)?cm:-1;
 tierCol=radarColor(tier);
}
buildRadar();requestAnimationFrame(tick);
function fmtUptime(ms){const s=Math.floor(ms/1000),d=Math.floor(s/86400),h=Math.floor(s%86400/3600),m=Math.floor(s%3600/60),sec=s%60;return d?`${d}d ${h}h ${m}m`:`${h}h ${m}m ${sec}s`}
function tierClass(t){return t==='DANGER'?'bad':t==='WARN'?'warn':'ok'}
function badgeFor(cm,t){if(cm<0) return '<span class="badge safe">no echo</span>';if(t==='DANGER') return '<span class="badge danger">DANGER</span>';if(t==='WARN') return '<span class="badge warn">WARN</span>';return '<span class="badge safe">SAFE</span>'}
let lastSensors=[];
const rowRefs=[];
const ACTION_BTN_IDS=['toggleBtn','resetBtn','thBtn','timeBtn','calBtn'];
const WATCHDOG_MS=3000;
const PUSH_STALE_MS=6000;
let es=null,lastPushAt=0,firstData=false;

document.addEventListener('input',e=>{if(e.target.tagName==='INPUT'&&e.target.type==='number')e.target.dataset.dirty='1'});
function clearDirty(ids){ids.forEach(id=>{const el=document.getElementById(id);if(el)delete el.dataset.dirty})}
function setInputClean(id,val){
 const el=document.getElementById(id);
 if(!el||el.dataset.dirty||document.activeElement===el)return;
 const s=String(val);if(el.value!==s)el.value=s;
}
function ensureRows(count){
 while(rowRefs.length<count){
  const i=rowRefs.length,tr=document.createElement('tr');
   const mk=(label,inner)=>{const td=document.createElement('td');td.dataset.l=label;if(inner!==undefined){if(inner instanceof Node)td.appendChild(inner);else td.innerHTML=inner}return td};
  const rawTd=mk('Raw'),cmTd=mk('Corrected'),stateTd=mk('State');
  const numIn=(id,label,step,min,max,w)=>{const inp=document.createElement('input');inp.id=id;inp.type='number';inp.step=step;inp.min=min;inp.max=max;inp.placeholder='—';inp.style.cssText=`width:${w}px;background:var(--card2);border:1px solid var(--line);color:var(--text);border-radius:8px;padding:6px 8px`;return mk(label,inp)};
  const off=numIn('off'+i,'Offset','0.1','-50','50',90);
  const scale=numIn('scale'+i,'Scale','0.05','0.5','2',80);
  const dly=numIn('dly'+i,'Delay ms','10','10','5000',70);
  const pwrBtn=document.createElement('button');
  pwrBtn.textContent='ON';
  pwrBtn.addEventListener('click',()=>toggleSensor(i,pwrBtn));
  const pwrTd=mk('Power');pwrTd.appendChild(pwrBtn);
  tr.append(mk('#',i+1),rawTd,cmTd,off,scale,dly,stateTd,pwrTd);
  document.getElementById('sensorBody').appendChild(tr);
  rowRefs.push({tr,rawTd,cmTd,stateTd,pwrBtn,off:off.firstChild,scale:scale.firstChild,dly:dly.firstChild});
 }
}
function renderSensors(j){
 lastSensors=j.sensors||[];
 ensureRows(lastSensors.length);
 lastSensors.forEach((s,i)=>{
  const r=rowRefs[i];
  r.tr.classList.toggle('rowOff',s.enabled===false);
  r.rawTd.innerHTML=s.raw>=0?s.raw.toFixed(1)+' cm':(s.enabled===false?'—':'<span class="badge danger" title="'+(s.status||'fail')+'">FAILED</span>');
  r.cmTd.innerHTML=s.cm>=0?s.cm.toFixed(1)+' cm':'—';
  r.stateTd.innerHTML=s.enabled===false?'—':(s.cm>=0?badgeFor(s.cm,j.tier):'<span class="badge danger" title="read failed">FAILED</span>');
  const on=s.enabled!==false;
  r.pwrBtn.setAttribute('style',`padding:5px 10px;border-radius:999px;font-size:11px;font-weight:700;cursor:pointer;border:1px solid ${on?'rgba(93,211,158,.4)':'var(--line)'};background:${on?'rgba(93,211,158,.12)':'transparent'};color:${on?'var(--ok)':'var(--muted)'}`);
  if(r.pwrBtn.textContent!==(on?'ON':'OFF'))r.pwrBtn.textContent=on?'ON':'OFF';
  setInputClean('off'+i,s.offset);
  setInputClean('scale'+i,s.scale);
  setInputClean('dly'+i,s.delay_ms||10);
 });
}
function render(j,latMs){
 document.getElementById('nearest').textContent=j.nearest_cm>=0?j.nearest_cm.toFixed(1):'—';
 const tb=document.getElementById('tierBadge');tb.textContent=j.tier;tb.className='tierBadge '+tierClass(j.tier);
 const btn=document.getElementById('toggleBtn');
 const lbl=j.running?'STOP':'START';
 if(btn.textContent!==lbl){btn.textContent=lbl;btn.className='btn '+(j.running?'btn-stop':'btn-start')}
 radarRun=!!j.running;
 if(j.sample_interval_ms)curSample=j.sample_interval_ms;
 updateRadar(j.nearest_cm,j.tier);
 const wifiEl=document.getElementById('wifi');wifiEl.textContent=j.wifi_connected?'connected':'offline';wifiEl.className=j.wifi_connected?'ok':'bad';
 document.getElementById('ip').textContent=j.ip;
 document.getElementById('rssi').textContent=j.rssi_dbm+' dBm';
 document.getElementById('heap').textContent=(j.free_heap/1024).toFixed(0)+' KB';
 document.getElementById('uptime').textContent=fmtUptime(j.uptime_ms);
 if(latMs!==undefined)document.getElementById('latency').textContent=latMs+' ms';
 setInputClean('dangerIn',j.danger_cm);
 setInputClean('warnIn',j.warn_cm);
 setInputClean('intervalIn',j.sample_interval_ms);
 setInputClean('timeoutIn',j.echo_timeout_ms);
 renderSensors(j);
 if(!firstData){firstData=true;ACTION_BTN_IDS.forEach(id=>{document.getElementById(id).disabled=false})}
}
async function postJson(url,body){
 const r=await fetch(url,{method:'POST',headers:{'Content-Type':'application/json'},body:body===undefined?undefined:JSON.stringify(body)});
 if(!r.ok)throw new Error('HTTP '+r.status);
 return r.json();
}
async function runAction(btn,msgEl,send,clearIds=[]){
 const orig=btn.textContent,origCls=btn.className;
 showSync();
 btn.disabled=true;btn.textContent='';btn.appendChild(Object.assign(document.createElement('span'),{className:'spin'}));
 let ok=false;
 try{
  const j=await send();
  render(j);
  clearDirty(clearIds);
  ok=true;
  if(msgEl){msgEl.textContent='Applied';setTimeout(()=>msgEl.textContent='',1500)}
 }catch(e){if(msgEl)msgEl.textContent='Failed'}
 finally{
  btn.disabled=false;
  if(!ok||btn.textContent===''){btn.textContent=orig;btn.className=origCls}
 }
}
function numVal(id){return parseFloat(document.getElementById(id).value)}
async function toggleEngine(){runAction(document.getElementById('toggleBtn'),null,()=>postJson('/api/toggle'))}
async function applyThresholds(){
 const d=numVal('dangerIn'),w=numVal('warnIn');
 if(isNaN(d)||isNaN(w)){document.getElementById('thMsg').textContent='No data yet';return}
 runAction(document.getElementById('thBtn'),document.getElementById('thMsg'),()=>postJson('/api/config',{danger_cm:d,warn_cm:w}),['dangerIn','warnIn']);
}
async function applyTiming(){
 const iv=numVal('intervalIn'),to=numVal('timeoutIn');
 if(isNaN(iv)||isNaN(to)){document.getElementById('timeMsg').textContent='No data yet';return}
 runAction(document.getElementById('timeBtn'),document.getElementById('timeMsg'),()=>postJson('/api/config',{sample_interval_ms:iv,echo_timeout_ms:to}),['intervalIn','timeoutIn']);
}
function buildSensorPayload(){
 return lastSensors.map((s,i)=>{
  const off=parseFloat(rowRefs[i].off.value),sc=parseFloat(rowRefs[i].scale.value),dl=parseFloat(rowRefs[i].dly.value);
  return{offset:isNaN(off)?s.offset:off,scale:isNaN(sc)?s.scale:sc,delay_ms:isNaN(dl)?(s.delay_ms||10):dl,enabled:s.enabled!==false};
 });
}
function sensorInputIds(){return lastSensors.flatMap((_,i)=>['off'+i,'scale'+i,'dly'+i])}
async function applyCalibration(){
 if(!lastSensors.length)return;
 runAction(document.getElementById('calBtn'),document.getElementById('calMsg'),()=>postJson('/api/config',{sensors:buildSensorPayload()}),sensorInputIds());
}
async function doReset(){
 runAction(document.getElementById('resetBtn'),null,()=>postJson('/api/reset'),['dangerIn','warnIn','intervalIn','timeoutIn',...sensorInputIds()]);
}
async function toggleSensor(i,btnEl){
 if(!lastSensors[i])return;
 const payload=buildSensorPayload();
 payload[i].enabled=!payload[i].enabled;
 await runAction(btnEl,null,()=>postJson('/api/config',{sensors:payload}));
}
async function fetchOnce(){
 try{
  const t0=performance.now();
  const j=await(await fetch('/api/status')).json();
  render(j,Math.round(performance.now()-t0));
  hideSync();
 }catch(e){}
}
async function startWatchdog(){
 setInterval(async()=>{
  if(es&&Date.now()-lastPushAt>PUSH_STALE_MS&&es.readyState===EventSource.CONNECTING){es.close();es=null}
  if(!es&&!startLive())await fetchOnce();
 },WATCHDOG_MS);
}
function startLive(){
 try{es=new EventSource('/api/events')}catch(e){es=null;return false}
 es.onmessage=e=>{
  hideSync();
  const now=Date.now();
  if(lastPushAt)document.getElementById('latency').textContent=(now-lastPushAt)+' ms';
  lastPushAt=now;
  try{render(JSON.parse(e.data))}catch(err){}
 };
 es.onerror=()=>{
  if(es.readyState===EventSource.CLOSED){es=null;fetchOnce()}
 };
 return true;
}
(async()=>{
 await fetchOnce();
 startWatchdog();
 if(!startLive())setInterval(fetchOnce,1000);
})();
</script>
</body>
</html>)rawliteral";
