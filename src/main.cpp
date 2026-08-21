#include <Arduino.h>
#include <WebServer.h>
#include <Log.h>
#include <OTA.h>
#include <MWIT_WiFi.h>
#include "credentials.h"
#include "webui.h"

MWITWiFi wifi(WIFI_SSID, WIFI_IDENTITY, WIFI_USERNAME, WIFI_PASSWORD);
OTA ota("miniproject-ota");
WebServer server(80);

String buildStatusJson() {
    String json = "{";
    json += "\"uptime_ms\":" + String(millis());
    json += ",\"wifi_connected\":" + String(wifi.connected() ? "true" : "false");
    json += ",\"ip\":\"" + wifi.localIP().toString() + "\"";
    json += ",\"rssi_dbm\":" + String(wifi.rssi());
    json += ",\"free_heap\":" + String(ESP.getFreeHeap());
    json += "}";
    return json;
}

void handleRoot() {
    server.send_P(200, "text/html", WEBUI_PAGE);
}

void handleStatus() {
    server.send(200, "application/json", buildStatusJson());
}

void handleNotFound() {
    server.send(404, "text/plain", "Not found");
}

void setup() {
    Logger.begin(115200);
    Logger.println("=== MiniProject: Standalone Alarm ===");

    wifi.begin();
    ota.begin();

    server.on("/", HTTP_GET, handleRoot);
    server.on("/api/status", HTTP_GET, handleStatus);
    server.onNotFound(handleNotFound);
    server.begin();

    if (wifi.connected()) {
        Logger.printf("[WEB] http://%s/\n", wifi.localIP().toString().c_str());
    } else {
        Logger.println("[WEB] WiFi offline — alarm core runs local-only");
    }
}

void loop() {
    Logger.handle();
    ota.handle();
    server.handleClient();
}
