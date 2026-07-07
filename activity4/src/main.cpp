#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <esp_wpa2.h>

#define DHTPIN 4
#define DHTTYPE DHT22

const char* ssid = "MWIT-WiFi";
const char* eap_identity = "s6709222";
const char* eap_username = "s6709222";
const char* eap_password = "g++-std=c++17";

DHT dht(DHTPIN, DHTTYPE);
WiFiServer server(80);

float computeHeatIndex(float t, float h) {
  if (t < 26.7 || h < 40) return t;
  float tf = t * 9.0 / 5.0 + 32.0;
  float hi = -42.379 + 2.04901523*tf + 10.14333127*h
             - 0.22475541*tf*h - 0.00683783*tf*tf
             - 0.05481717*h*h + 0.00122874*tf*tf*h
             + 0.00085282*tf*h*h - 0.00000199*tf*tf*h*h;
  if (hi < tf) return t;
  return (hi - 32.0) * 5.0 / 9.0;
}

const char* HTML_PAGE = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Heat Index Monitor</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{
  font-family:Inter,-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;
  min-height:100vh;min-height:100dvh;
  background:linear-gradient(135deg,#0f0c29,#302b63,#24243e);
  display:flex;justify-content:center;align-items:center;
  padding:max(12px,1.5vmin)
}
.dashboard{width:100%;max-width:min(1400px,98vw)}

/* === HERO: FEELS LIKE === */
.hero{
  background:rgba(255,255,255,.07);
  backdrop-filter:blur(24px);-webkit-backdrop-filter:blur(24px);
  border:1px solid rgba(255,255,255,.1);
  border-radius:clamp(20px,3vmin,36px);
  padding:clamp(18px,3.5vmin,42px) clamp(20px,4vmin,48px);
  box-shadow:0 30px 60px -15px rgba(0,0,0,.6);
  position:relative;overflow:hidden;margin-bottom:clamp(8px,1.5vmin,20px);
  display:flex;align-items:center;justify-content:space-between;
  gap:clamp(16px,3vmin,40px)
}
.hero::before{
  content:'';position:absolute;top:0;left:0;right:0;height:1px;
  background:linear-gradient(90deg,transparent,rgba(255,255,255,.3),transparent)
}
.hero::after{
  content:'';position:absolute;top:-50%;left:-50%;width:200%;height:200%;
  background:radial-gradient(ellipse at 20% 50%,rgba(251,191,36,.06) 0%,transparent 60%);
  pointer-events:none
}
.hero-main{display:flex;flex-direction:column;gap:clamp(2px,.3vmin,6px);z-index:1}
.hero-label{
  color:rgba(255,255,255,.4);
  font-size:clamp(.6rem,1.1vw,.9rem);
  font-weight:700;text-transform:uppercase;
  letter-spacing:.35em
}
.hero-temp{
  font-size:clamp(3.5rem,16vw,9rem);
  font-weight:900;
  letter-spacing:-.04em;line-height:1;
  transition:color .5s ease
}
.hero-unit{
  color:rgba(255,255,255,.3);
  font-size:clamp(1.2rem,5vw,3rem);
  font-weight:500;margin-left:.06em;vertical-align:super
}
/* === HERO RIGHT: COMPACT STATS === */
.hero-stats{
  display:flex;flex-direction:column;gap:clamp(6px,1vmin,16px);
  flex-shrink:0;z-index:1
}
.stat-row{
  display:flex;align-items:center;gap:clamp(8px,1.5vmin,20px);
  background:rgba(255,255,255,.04);
  border:1px solid rgba(255,255,255,.06);
  border-radius:clamp(10px,1.5vmin,18px);
  padding:clamp(8px,1.2vmin,16px) clamp(12px,2vmin,24px)
}
.stat-item{text-align:center}
.stat-item-label{
  color:rgba(255,255,255,.3);
  font-size:clamp(.45rem,.7vw,.6rem);
  font-weight:600;text-transform:uppercase;
  letter-spacing:.2em;margin-bottom:2px
}
.stat-item-value{
  color:rgba(255,255,255,.85);
  font-size:clamp(.9rem,1.8vw,1.4rem);
  font-weight:700
}
.stat-item-unit{
  color:rgba(255,255,255,.3);
  font-size:clamp(.5rem,1vw,.75rem);
  font-weight:400;margin-left:2px
}
.stat-divider{
  width:1px;height:clamp(24px,3.5vmin,42px);
  background:rgba(255,255,255,.08);flex-shrink:0
}
.delta{
  text-align:center;min-width:clamp(50px,8vw,110px)
}
.delta-value{
  font-size:clamp(1.2rem,2.5vw,2rem);
  font-weight:800;line-height:1
}
.delta-value.plus{color:#f97316}
.delta-value.minus{color:#38bdf8}
.delta-value.zero{color:rgba(255,255,255,.3)}
.delta-label{
  color:rgba(255,255,255,.25);
  font-size:clamp(.4rem,.6vw,.5rem);
  font-weight:600;text-transform:uppercase;
  letter-spacing:.15em;margin-top:1px
}

/* === GRID === */
.grid{display:grid;grid-template-columns:1fr 1fr;gap:clamp(8px,1.5vmin,20px)}
.card{
  background:rgba(255,255,255,.06);
  backdrop-filter:blur(20px);-webkit-backdrop-filter:blur(20px);
  border:1px solid rgba(255,255,255,.1);
  border-radius:clamp(16px,2.5vmin,28px);
  padding:clamp(14px,2.5vmin,32px) clamp(12px,2vmin,28px);
  box-shadow:0 25px 50px -12px rgba(0,0,0,.5);
  position:relative;overflow:hidden;
  display:flex;flex-direction:column;
  align-items:center;justify-content:center;
  min-height:clamp(80px,20vmin,240px)
}
.card::before{
  content:'';position:absolute;top:0;left:0;right:0;height:1px;
  background:linear-gradient(90deg,transparent,rgba(255,255,255,.3),transparent)
}
.card-icon{font-size:clamp(1.3rem,3.5vw,2.8rem);display:block;margin-bottom:clamp(2px,.5vmin,8px)}
.card-label{
  color:rgba(255,255,255,.35);
  font-size:clamp(.5rem,1vw,.75rem);
  font-weight:600;text-transform:uppercase;
  letter-spacing:.25em;margin-bottom:clamp(2px,.5vmin,8px)
}
.card-value{
  color:#fff;
  font-size:clamp(2rem,8vw,4.5rem);
  font-weight:800;
  letter-spacing:-.03em;line-height:1;
  transition:color .5s ease
}
.card-unit{
  color:rgba(255,255,255,.25);
  font-size:clamp(.75rem,2.5vw,1.5rem);
  font-weight:500;margin-left:.06em;vertical-align:super
}
.temp-value.cool{color:#60a5fa}
.temp-value.warm{color:#fbbf24}
.temp-value.hot{color:#ef4444}
.humid-bar-track{
  width:100%;max-width:280px;height:clamp(4px,.7vmin,10px);
  background:rgba(255,255,255,.08);
  border-radius:99px;margin-top:clamp(6px,1.5vmin,14px);overflow:hidden
}
.humid-bar-fill{
  height:100%;border-radius:99px;
  background:linear-gradient(90deg,#38bdf8,#818cf8);
  transition:width .6s cubic-bezier(.4,0,.2,1)
}

/* === FOOTER === */
.footer{
  display:flex;justify-content:space-between;align-items:center;
  margin-top:clamp(6px,1vmin,12px);
  padding:0 clamp(2px,.3vmin,8px)
}
.status{display:flex;align-items:center;gap:clamp(4px,.5vmin,8px)}
.dot{width:clamp(5px,.6vmin,10px);height:clamp(5px,.6vmin,10px);border-radius:50%;flex-shrink:0}
.dot.online{background:#22c55e;box-shadow:0 0 clamp(4px,.8vmin,12px) #22c55e}
.dot.offline{background:#ef4444;box-shadow:0 0 clamp(4px,.8vmin,12px) #ef4444}
.label{
  color:rgba(255,255,255,.25);
  font-size:clamp(.45rem,.7vw,.6rem);
  font-weight:600;letter-spacing:.12em
}
.last-update{
  color:rgba(255,255,255,.15);
  font-size:clamp(.4rem,.6vw,.55rem);
  font-weight:500;letter-spacing:.1em
}
@media(max-width:700px){
  .hero{flex-direction:column;text-align:center}
  .stat-row{flex-wrap:wrap;justify-content:center}
  .grid{grid-template-columns:1fr}
  .card{min-height:clamp(80px,20vw,140px)}
}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.4}}
.loading .hero-temp{animation:pulse 1.2s ease-in-out infinite;color:rgba(255,255,255,.12)}
</style>
</head>
<body>
<div class="dashboard">
  <!-- FEELS LIKE HERO -->
  <div class="hero">
    <div class="hero-main">
      <div class="hero-label">Feels Like</div>
      <div class="hero-temp" id="hiValue">--<span class="hero-unit">&deg;C</span></div>
    </div>
    <div class="hero-stats">
      <div class="stat-row">
        <div class="stat-item">
          <div class="stat-item-label">Actual</div>
          <div class="stat-item-value">
            <span class="temp-value" id="actualTempValue">--<span class="stat-item-unit">&deg;C</span></span>
          </div>
        </div>
        <div class="stat-divider"></div>
        <div class="stat-item">
          <div class="stat-item-label">Humidity</div>
          <div class="stat-item-value">
            <span id="actualHumidValue">--<span class="stat-item-unit">%</span></span>
          </div>
        </div>
        <div class="stat-divider"></div>
        <div class="delta">
          <div class="delta-value" id="deltaValue">+0.0&deg;</div>
          <div class="delta-label">Difference</div>
        </div>
      </div>
    </div>
  </div>
  <!-- BOTTOM GRID -->
  <div class="grid">
    <div class="card">
      <span class="card-icon">&#x1f321;</span>
      <div class="card-label">Actual Temperature</div>
      <div class="card-value temp-value" id="tempValue">--<span class="card-unit">&deg;C</span></div>
    </div>
    <div class="card">
      <span class="card-icon">&#x1f4a7;</span>
      <div class="card-label">Humidity</div>
      <div class="card-value" id="humidValue">--<span class="card-unit">%</span></div>
      <div class="humid-bar-track"><div class="humid-bar-fill" id="humidBar" style="width:0%"></div></div>
    </div>
  </div>
  <!-- FOOTER -->
  <div class="footer">
    <div class="status"><div class="dot online" id="statusDot"></div><span class="label" id="statusText">ONLINE</span></div>
    <span class="label" id="deviceIP"></span>
    <span class="last-update" id="lastUpdate">--</span>
  </div>
</div>
<script>
const E={
  hi:document.getElementById('hiValue'),
  atv:document.getElementById('actualTempValue'),
  ahv:document.getElementById('actualHumidValue'),
  dv:document.getElementById('deltaValue'),
  temp:document.getElementById('tempValue'),
  humid:document.getElementById('humidValue'),
  bar:document.getElementById('humidBar'),
  dot:document.getElementById('statusDot'),
  st:document.getElementById('statusText'),
  lu:document.getElementById('lastUpdate'),
  ip:document.getElementById('deviceIP')
};
let lt=null,lh=null,lhi=null;

function tc(t){return t>=35?'hot':t>=28?'warm':'cool'}

function anim(el,f,t,u,isDelta){
  if(f===null){el.innerHTML=t>0&&isDelta?'+'+t.toFixed(1)+'<span class="stat-item-unit">&deg;</span>':t.toFixed(1)+'<span class="'+(!isDelta?'card-unit':'stat-item-unit')+'">'+u+'</span>';return}
  const s=performance.now(),d=400;
  function r(n){
    const p=Math.min((n-s)/d,1),e=1-Math.pow(1-p,3),c=f+(t-f)*e;
    let txt;
    if(isDelta) txt=(c>0?'+':'')+c.toFixed(1)+'<span class="stat-item-unit">&deg;</span>';
    else if(el===E.hi) txt=c.toFixed(1)+'<span class="hero-unit">'+u+'</span>';
    else txt=c.toFixed(1)+'<span class="'+(el===E.temp||el===E.humid?'card-unit':(el===E.atv||el===E.ahv?'stat-item-unit':''))+'">'+u+'</span>';
    el.innerHTML=txt;
    if(p<1)requestAnimationFrame(r)
  }
  requestAnimationFrame(r)
}

async function fetchData(){
  try{
    const r=await fetch('/data');
    if(!r.ok)throw new Error(r.status);
    const d=await r.json();
    if(d.error)throw new Error(d.error);
    const hi=d.heatIndex||d.temperature;
    const delta=hi-d.temperature;
    anim(E.hi,lhi,hi,'°C',false);
    const cc={'cool':'#60a5fa','warm':'#fbbf24','hot':'#ef4444'}[tc(hi)];
    E.hi.style.color=cc;
    anim(E.temp,lt,d.temperature,'°C',false);
    E.temp.className='card-value temp-value '+tc(d.temperature);
    E.atv.innerHTML=d.temperature.toFixed(1)+'<span class="stat-item-unit">&deg;C</span>';
    E.atv.className='temp-value '+tc(d.temperature);
    anim(E.humid,lh,d.humidity,'%',false);
    E.bar.style.width=Math.min(d.humidity,100)+'%';
    E.ahv.innerHTML=d.humidity.toFixed(1)+'<span class="stat-item-unit">%</span>';
    const dc=delta>0.3?'plus':delta<-0.3?'minus':'zero';
    E.dv.className='delta-value '+dc;
    E.dv.innerHTML=(delta>0?'+':'')+delta.toFixed(1)+'<span class="stat-item-unit">&deg;</span>';
    lt=d.temperature;lh=d.humidity;lhi=hi;
    E.lu.textContent=new Date().toLocaleTimeString();
    E.dot.className='dot online';E.st.textContent='ONLINE';
    E.ip.textContent=d.ip||'';
  }catch(e){
    E.dot.className='dot offline';E.st.textContent='OFFLINE';
  }
}
fetchData();setInterval(fetchData,2000);
</script>
</body>
</html>
)rawliteral";

void handleClient(WiFiClient& client) {
  String method, path;
  bool readingPath = false;
  bool firstLine = true;

  while (client.connected()) {
    if (!client.available()) continue;
    String line = client.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) break;

    if (firstLine) {
      int space1 = line.indexOf(' ');
      int space2 = line.indexOf(' ', space1 + 1);
      if (space1 > 0 && space2 > space1) {
        method = line.substring(0, space1);
        path = line.substring(space1 + 1, space2);
      }
      firstLine = false;
    }
  }

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (path == "/data") {
    float hi = computeHeatIndex(t, h);
    String json = "{\"temperature\":" + String(t) +
                  ",\"humidity\":" + String(h) +
                  ",\"heatIndex\":" + String(hi) +
                  ",\"ip\":\"" + WiFi.localIP().toString() +
                  "\"}";
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(json.length());
    client.println("Connection: close");
    client.println();
    client.print(json);
  } else {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.print(HTML_PAGE);
  }

  delay(10);
  client.stop();
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  Serial.print("Web server started at http://");
  Serial.println(WiFi.localIP());
}

void WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  uint8_t reason = info.wifi_sta_disconnected.reason;
  Serial.print("\nDisconnected! Reason: ");
  switch (reason) {
    case 1:  Serial.println("1 - Unspecified"); break;
    case 2:  Serial.println("2 - Auth expired"); break;
    case 6:  Serial.println("6 - Not authenticated"); break;
    case 15: Serial.println("15 - 4-way handshake timeout"); break;
    case 19: Serial.println("19 - No AP found (check SSID)"); break;
    case 22: Serial.println("22 - 802.1X auth failed"); break;
    case 23: Serial.println("23 - Cipher rejected"); break;
    case 24: Serial.println("24 - Beacon timeout"); break;
    case 25: Serial.println("25 - No AP found"); break;
    case 201: Serial.println("201 - No SSID found"); break;
    case 202: Serial.println("202 - Auth failed"); break;
    default: Serial.println(reason); break;
  }
  delay(3000);
  WiFi.reconnect();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);

  Serial.println("Scanning for networks...");
  int n = WiFi.scanNetworks();
  Serial.print("Found ");
  Serial.print(n);
  Serial.println(" networks:");

  bool targetFound = false;
  for (int i = 0; i < n; i++) {
    Serial.print("  ");
    Serial.print(i + 1);
    Serial.print(". ");
    Serial.print(WiFi.SSID(i));
    Serial.print(" (");
    Serial.print(WiFi.RSSI(i));
    Serial.println(" dBm)");
    if (WiFi.SSID(i) == ssid) targetFound = true;
  }

  if (!targetFound) {
    Serial.print("\nTarget SSID \"");
    Serial.print(ssid);
    Serial.println("\" not found!");
    return;
  }

  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  esp_wifi_sta_wpa2_ent_set_identity((uint8_t*)eap_identity, strlen(eap_identity));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t*)eap_username, strlen(eap_username));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t*)eap_password, strlen(eap_password));
  esp_wifi_sta_wpa2_ent_set_disable_time_check(true);
  esp_wifi_sta_wpa2_ent_enable();

  Serial.print("\nConnecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid);
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    handleClient(client);
  }
}
