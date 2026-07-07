#include <Arduino.h>
#include <WiFi.h>
#include <esp_wpa2.h>
#include <SPIFFS.h>
#include "credentials.h"

#define NUM_LEDS 3

const int ledPins[NUM_LEDS] = {2, 4, 5};
int ledValues[NUM_LEDS] = {0, 0, 0};

const char* ssid = WIFI_SSID;
const char* eap_identity = WIFI_IDENTITY;
const char* eap_username = WIFI_USERNAME;
const char* eap_password = WIFI_PASSWORD;

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

// ====== Content-Type helper ======

String getContentType(String path) {
  if (path.endsWith(".html")) return "text/html";
  if (path.endsWith(".css"))  return "text/css";
  if (path.endsWith(".js"))   return "application/javascript";
  if (path.endsWith(".json")) return "application/json";
  if (path.endsWith(".png"))  return "image/png";
  if (path.endsWith(".jpg") || path.endsWith(".jpeg")) return "image/jpeg";
  if (path.endsWith(".svg"))  return "image/svg+xml";
  if (path.endsWith(".ico"))  return "image/x-icon";
  return "text/plain";
}

// ====== Serve static file ======

bool serveFile(WiFiClient& client, String path) {
  if (path == "/") path = "/index.html";
  if (!SPIFFS.exists(path)) return false;

  File file = SPIFFS.open(path, "r");
  if (!file) return false;

  client.println("HTTP/1.1 200 OK");
  client.print("Content-Type: "); client.println(getContentType(path));
  client.println("Connection: close");
  client.println();

  uint8_t buf[512];
  int r;
  while ((r = file.read(buf, sizeof(buf))) > 0) client.write(buf, r);
  file.close();
  return true;
}

// ====== Send JSON response helper ======

void sendJson(WiFiClient& client, const String& json) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.print("Content-Length: "); client.println(json.length());
  client.println("Connection: close"); client.println();
  client.print(json);
}

// ====== API: set LED brightness ======

void setLed(WiFiClient& client, int idx, int value) {
  value = constrain(value, 0, 255);
  ledValues[idx] = value;
  ledcWrite(idx, value);
  String json = "{\"ok\":true,\"led\":" + String(idx + 1) + ",\"brightness\":" + String(value) + "}";
  sendJson(client, json);
}

void handleApiLed(WiFiClient& client, String& path) {
  for (int i = 0; i < NUM_LEDS; i++) {
    String base = "/api/led/" + String(i + 1);

    if (path == base + "/on")   { setLed(client, i, 255); return; }
    if (path == base + "/off")  { setLed(client, i, 0);   return; }

    if (path == base + "/toggle") {
      setLed(client, i, ledValues[i] > 0 ? 0 : 255);
      return;
    }

    if (path.startsWith(base + "/set")) {
      int q = path.indexOf('?');
      if (q < 0) { setLed(client, i, 255); return; }
      String qs = path.substring(q + 1);
      int bp = qs.indexOf("b=");
      if (bp < 0) { setLed(client, i, 255); return; }
      String vs = qs.substring(bp + 2);
      int ve = 0;
      while (ve < (int)vs.length() && vs[ve] >= '0' && vs[ve] <= '9') ve++;
      int val = vs.substring(0, ve).toInt();
      setLed(client, i, val);
      return;
    }
  }

  // 404 if we get here (no matching LED)
  client.println("HTTP/1.1 404 Not Found");
  client.println("Connection: close");
  client.println();
  client.stop();
}

// ====== API: status ======

void handleStatus(WiFiClient& client) {
  String json = "{\"leds\":[";
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i > 0) json += ",";
    json += String(ledValues[i]);
  }
  json += "],\"ip\":\"" + WiFi.localIP().toString() + "\"}";
  sendJson(client, json);
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

  if (path.startsWith("/api/led/")) {
    handleApiLed(client, path);
    return;
  }

  if (path == "/api/status") {
    handleStatus(client);
    return;
  }

  if (!serveFile(client, path)) {
    Serial.print("FILE NOT FOUND in SPIFFS: "); Serial.println(path);
    client.println("HTTP/1.1 404 Not Found");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE html><body style='font-family:sans-serif;padding:40px'>");
    client.println("<h2>404 - File Not Found</h2>");
    client.print("<p>Path: <code>"); client.print(path); client.println("</code></p>");
    client.println("<p>Did you run <code>pio run -t uploadfs</code>?</p>");
    client.println("</body>");
  }
  client.stop();
}

// ====== setup ======

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Attach PWM LEDC channels (5kHz, 8-bit resolution)
  for (int i = 0; i < NUM_LEDS; i++) {
    ledcSetup(i, 5000, 8);
    ledcAttachPin(ledPins[i], i);
    ledcWrite(i, 0);
  }

  // Mount SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed!");
    return;
  }
  Serial.println("SPIFFS mounted");

  // List files for debug
  File root = SPIFFS.open("/");
  if (root && root.isDirectory()) {
    File f = root.openNextFile();
    while (f) {
      Serial.print("  FILE: /"); Serial.print(f.name());
      Serial.print(" ("); Serial.print(f.size()); Serial.println(" bytes)");
      f = root.openNextFile();
    }
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
