#include <Arduino.h>
#include <WiFi.h>
#include <esp_wpa2.h>

#define NUM_LEDS 3

const int ledPins[NUM_LEDS] = {2, 4, 5};
bool ledStates[NUM_LEDS] = {false, false, false};
const char* ledColors[NUM_LEDS] = {"blue", "white", "yellow"};

const char* ssid = "MWIT-WiFi";
const char* eap_identity = "s6709222";
const char* eap_username = "s6709222";
const char* eap_password = "g++-std=c++17";

WiFiServer server(80);

// ====== WiFi ======

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  Serial.print("Web server at http://");
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
    case 19: Serial.println("19 - No AP found"); break;
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

// ====== API handlers ======

void handleLedToggle(WiFiClient& client, String& path) {
  for (int i = 0; i < NUM_LEDS; i++) {
    String onStr  = "/api/led/" + String(i + 1) + "/on";
    String offStr = "/api/led/" + String(i + 1) + "/off";

    if (path == onStr) {
      digitalWrite(ledPins[i], HIGH);
      ledStates[i] = true;
      Serial.printf("LED%d ON (GPIO%d)\n", i + 1, ledPins[i]);
      String json = "{\"ok\":true,\"led\":" + String(i + 1) + ",\"state\":true}";
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/json");
      client.print("Content-Length: "); client.println(json.length());
      client.println("Connection: close"); client.println();
      client.print(json);
      client.stop();
      return;
    }

    if (path == offStr) {
      digitalWrite(ledPins[i], LOW);
      ledStates[i] = false;
      Serial.printf("LED%d OFF (GPIO%d)\n", i + 1, ledPins[i]);
      String json = "{\"ok\":true,\"led\":" + String(i + 1) + ",\"state\":false}";
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/json");
      client.print("Content-Length: "); client.println(json.length());
      client.println("Connection: close"); client.println();
      client.print(json);
      client.stop();
      return;
    }
  }
}

void handleStatus(WiFiClient& client) {
  String json = "{\"leds\":[";
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i > 0) json += ",";
    json += ledStates[i] ? "true" : "false";
  }
  json += "],\"ip\":\"" + WiFi.localIP().toString() + "\"}";

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.print("Content-Length: "); client.println(json.length());
  client.println("Connection: close"); client.println();
  client.print(json);
  client.stop();
}

// ====== Serve HTML page ======

void servePage(WiFiClient& client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();

  client.println("<!DOCTYPE html>");
  client.println("<html lang=\"en\">");
  client.println("<head>");
  client.println("<meta charset=\"UTF-8\">");
  client.println("<meta name=\"viewport\" content=\"width=device-width,initial-scale=1.0\">");
  client.println("<title>ESP32 3-LED Control</title>");

  // -- CSS --
  client.println("<style>");
  client.println("*{margin:0;padding:0;box-sizing:border-box}");
  client.println("body{");
  client.println("  font-family:Inter,-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;");
  client.println("  min-height:100vh;min-height:100dvh;");
  client.println("  background:linear-gradient(135deg,#0f0c29,#302b63,#24243e);");
  client.println("  display:flex;justify-content:center;align-items:center;padding:16px");
  client.println("}");
  client.println(".card{");
  client.println("  background:rgba(255,255,255,.07);backdrop-filter:blur(24px);");
  client.println("  -webkit-backdrop-filter:blur(24px);");
  client.println("  border:1px solid rgba(255,255,255,.1);border-radius:24px;");
  client.println("  padding:32px;box-shadow:0 30px 60px -15px rgba(0,0,0,.6);");
  client.println("  max-width:700px;width:100%");
  client.println("}");
  client.println("h1{color:#fff;font-size:1.4rem;text-align:center;margin-bottom:4px;font-weight:700}");
  client.println(".sub{color:rgba(255,255,255,.3);font-size:.65rem;text-align:center;margin-bottom:24px}");

  // Realistic LED
  client.println(".led-row{display:flex;justify-content:center;gap:clamp(16px,4vw,40px)}");
  client.println(".led-col{display:flex;flex-direction:column;align-items:center;gap:4px}");
  client.println(".led-dome{");
  client.println("  width:38px;height:38px;border-radius:50% 50% 50% 50%/60% 60% 40% 40%;");
  client.println("  position:relative;transition:all .3s ease;border:1px solid rgba(255,255,255,.06)");
  client.println("}");
  client.println(".led-dome::after{");
  client.println("  content:'';position:absolute;top:5px;left:7px;");
  client.println("  width:11px;height:8px;background:rgba(255,255,255,.3);");
  client.println("  border-radius:50%;pointer-events:none");
  client.println("}");
  client.println(".led-dome.on.blue{background:#3b82f6;box-shadow:0 0 20px #3b82f6}");
  client.println(".led-dome.off.blue{background:#0f1a2d;border-color:rgba(59,130,246,.12)}");
  client.println(".led-dome.on.white{background:#f1f5f9;box-shadow:0 0 20px #f1f5f9}");
  client.println(".led-dome.off.white{background:#2a2a2a;border-color:rgba(241,245,249,.1)}");
  client.println(".led-dome.on.yellow{background:#eab308;box-shadow:0 0 20px #eab308}");
  client.println(".led-dome.off.yellow{background:#2d2408;border-color:rgba(234,179,8,.12)}");
  client.println(".led-base{width:12px;height:4px;background:#3a3a3a;border-radius:0 0 2px 2px}");
  client.println(".led-legs{display:flex;gap:5px}");
  client.println(".led-leg{width:2px;height:12px;background:#555;border-radius:1px}");
  client.println(".led-label{color:rgba(255,255,255,.35);font-size:.6rem;font-weight:600;letter-spacing:.1em;margin-top:2px}");

  // Buttons
  client.println(".btn-row{display:flex;gap:4px;margin-top:4px}");
  client.println(".btn{");
  client.println("  padding:6px 14px;border:none;border-radius:6px;font-size:.7rem;");
  client.println("  font-weight:600;cursor:pointer;color:#fff;transition:all .12s ease");
  client.println("}");
  client.println(".btn:active{transform:scale(.9)}");
  client.println(".btn:disabled{opacity:.3;cursor:not-allowed;transform:none}");
  client.println(".btn-on{background:#22c55e}");
  client.println(".btn-off{background:#ef4444}");

  // Footer
  client.println(".footer{display:flex;justify-content:space-between;align-items:center;margin-top:20px;padding-top:14px;border-top:1px solid rgba(255,255,255,.05)}");
  client.println(".dot{width:6px;height:6px;border-radius:50%;display:inline-block;vertical-align:middle}");
  client.println(".dot.online{background:#22c55e;box-shadow:0 0 8px #22c55e}");
  client.println(".ft-label{color:rgba(255,255,255,.2);font-size:.55rem}");
  client.println(".ft-ip{color:rgba(255,255,255,.12);font-size:.5rem}");
  client.println("</style>");
  client.println("</head><body>");

  // -- Body --
  client.println("<div class=\"card\">");
  client.println("<h1>ESP32 LED Control</h1>");
  client.println("<div class=\"sub\">3 independent LEDs &middot; REST API</div>");

  client.println("<div class=\"led-row\">");
  for (int i = 0; i < NUM_LEDS; i++) {
    client.print("<div class=\"led-col\">");

    // LED dome
    client.print("<div class=\"led-dome ");
    client.print(ledStates[i] ? "on" : "off");
    client.print(" ");
    client.print(ledColors[i]);
    client.println("\"></div>");

    // Base + legs
    client.println("<div class=\"led-base\"></div>");
    client.println("<div class=\"led-legs\"><div class=\"led-leg\"></div><div class=\"led-leg\"></div></div>");

    // Label
    client.print("<div class=\"led-label\">LED ");
    client.print(i + 1);
    client.println("</div>");

    // Buttons
    client.println("<div class=\"btn-row\">");
    client.print("<button class=\"btn btn-on\" id=\"on");
    client.print(i + 1);
    client.println("\">ON</button>");
    client.print("<button class=\"btn btn-off\" id=\"off");
    client.print(i + 1);
    client.println("\">OFF</button>");
    client.println("</div></div>");
  }
  client.println("</div>");

  // Footer
  client.println("<div class=\"footer\">");
  client.println("<span class=\"ft-label\"><span class=\"dot online\" id=\"statusDot\"></span> <span id=\"statusText\">ONLINE</span></span>");
  client.print("<span class=\"ft-ip\" id=\"ipDisplay\">");
  client.print(WiFi.localIP().toString());
  client.println("</span>");
  client.println("</div></div>");

  // -- JS --
  client.println("<script>");
  client.println("const sd=document.getElementById('statusDot'),st=document.getElementById('statusText');");

  // Build leds array
  client.print("const l=[");
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i > 0) client.print(",");
    client.print("{");
    client.print("d:document.querySelector('.led-col:nth-child("); client.print(i + 1); client.print(") .led-dome'),");
    client.print("b1:document.getElementById('on"); client.print(i + 1); client.print("'),");
    client.print("b0:document.getElementById('off"); client.print(i + 1); client.print("')");
    client.print("}");
  }
  client.println("];");

  client.println("async function t(n,s){try{await fetch('/api/led/'+n+'/'+(s?'on':'off'))}catch(_){}}");
  client.println("async function r(){try{const e=await fetch('/api/status'),d=await e.json();");
  client.println("d.leds.forEach((s,i)=>{l[i].d.className='led-dome '+(s?'on ':'off ')+['blue','white','yellow'][i];l[i].b1.disabled=s;l[i].b0.disabled=!s});");
  client.println("sd.className='dot online';st.textContent='ONLINE'}catch(e){sd.className='dot offline';st.textContent='OFFLINE'}}");
  client.println("for(let i=0;i<3;i++){l[i].b1.onclick=async()=>{l[i].b1.disabled=l[i].b0.disabled=1;await t(i+1,1);await r()};");
  client.println("l[i].b0.onclick=async()=>{l[i].b1.disabled=l[i].b0.disabled=1;await t(i+1,0);await r()}}");
  client.println("r();setInterval(r,2000);");
  client.println("</script>");
  client.println("</body></html>");
  client.println();
  client.stop();
}

// ====== Request dispatcher ======

void handleClient(WiFiClient& client) {
  String method, path;
  bool firstLine = true;

  while (client.connected()) {
    if (!client.available()) continue;
    String line = client.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) break;
    if (firstLine) {
      int s1 = line.indexOf(' ');
      int s2 = line.indexOf(' ', s1 + 1);
      if (s1 > 0 && s2 > s1) {
        method = line.substring(0, s1);
        path  = line.substring(s1 + 1, s2);
      }
      firstLine = false;
    }
  }

  Serial.print(method); Serial.print(" "); Serial.println(path);

  // API: /api/led/N/on  /api/led/N/off
  if (path.startsWith("/api/led/")) {
    handleLedToggle(client, path);
    return;
  }

  // API: /api/status
  if (path == "/api/status") {
    handleStatus(client);
    return;
  }

  // Serve HTML page
  servePage(client);
}

// ====== setup ======

void setup() {
  Serial.begin(115200);
  delay(1000);

  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);

  Serial.println("Scanning for WiFi networks...");
  int n = WiFi.scanNetworks();
  Serial.print("Found "); Serial.print(n); Serial.println(" networks:");

  bool targetFound = false;
  for (int i = 0; i < n; i++) {
    Serial.print("  "); Serial.print(i + 1); Serial.print(". ");
    Serial.print(WiFi.SSID(i)); Serial.print(" ("); Serial.print(WiFi.RSSI(i)); Serial.println(" dBm)");
    if (WiFi.SSID(i) == ssid) targetFound = true;
  }

  if (!targetFound) {
    Serial.print("\nTarget SSID \""); Serial.print(ssid); Serial.println("\" not found!");
    return;
  }

  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  esp_wifi_sta_wpa2_ent_set_identity((uint8_t*)eap_identity, strlen(eap_identity));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t*)eap_username, strlen(eap_username));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t*)eap_password, strlen(eap_password));
  esp_wifi_sta_wpa2_ent_set_disable_time_check(true);
  esp_wifi_sta_wpa2_ent_enable();

  Serial.print("\nConnecting to "); Serial.println(ssid);
  WiFi.begin(ssid);
}

// ====== loop ======

void loop() {
  WiFiClient client = server.available();
  if (client) {
    handleClient(client);
  }
}
