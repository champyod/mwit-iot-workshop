#include <Arduino.h>
#include <WebServer.h>
#include <Log.h>
#include <OTA.h>
#include <MWIT_WiFi.h>
#include <LED.h>
#include <GG_Sheet.h>
#include <DiscordWebhook.h>
#include "pins.h"
#include "credentials.h"
#include "webui.h"
#include "UltrasonicRCW.h"
#include "Button.h"
#include "ZoneEngine.h"

MWITWiFi wifi(WIFI_SSID, WIFI_IDENTITY, WIFI_USERNAME, WIFI_PASSWORD);
OTA ota("miniproject-ota");
WebServer server(80);

static UltrasonicRCW us1(PIN_US1_TRIG, PIN_US1_ECHO);
static UltrasonicRCW us2(PIN_US2_TRIG, PIN_US2_ECHO);
static Button engineButton(PIN_BUTTON);

static LED ledSafe(PIN_LED_SAFE);
static LED ledWarn(PIN_LED_WARN);
static LED ledDanger(PIN_LED_DANGER);
static LED buzzer(PIN_BUZZER);

static ZoneEngine engine;
static GGSheet sheets(SHEETS_HOST, SHEETS_PATH);
static DiscordWebhook discord(DISCORD_WEBHOOK_URL);

namespace {
constexpr unsigned long SHEET_LOG_INTERVAL_MS = 10000UL;
constexpr unsigned long DISCORD_COOLDOWN_MS   = 60000UL;
const char* RUN_LABEL = "danger-zone";
}

static const char* tierName(RiskTier tier) {
    switch (tier) {
    case RiskTier::DANGER: return "DANGER";
    case RiskTier::WARN:   return "WARN";
    case RiskTier::SAFE:   return "SAFE";
    }
    return "SAFE";
}

static String buildStatusJson() {
    String json = "{";
    json += "\"uptime_ms\":" + String(millis());
    json += ",\"wifi_connected\":" + String(wifi.connected() ? "true" : "false");
    json += ",\"ip\":\"" + wifi.localIP().toString() + "\"";
    json += ",\"rssi_dbm\":" + String(wifi.rssi());
    json += ",\"free_heap\":" + String(ESP.getFreeHeap());
    json += ",\"running\":" + String(engine.isRunning() ? "true" : "false");
    json += ",\"nearest_cm\":" + String(engine.nearestCm(), 1);
    json += ",\"tier\":\"" + String(tierName(engine.tier())) + "\"";
    json += "}";
    return json;
}

static void logToSheetsIfNeeded(bool forceRow) {
    static unsigned long lastSheetMs = 0;
    if (!wifi.connected() || !engine.isRunning()) return;

    const bool due = forceRow || (millis() - lastSheetMs >= SHEET_LOG_INTERVAL_MS);
    if (!due) return;
    lastSheetMs = millis();

    sheets.sendZone(RUN_LABEL, engine.nearestCm(), tierName(engine.tier()));
}

static void alertOnDangerEntry() {
    static unsigned long lastDiscordMs = 0;
    if (!engine.consumeTierChanged()) return;

    logToSheetsIfNeeded(true);  // immediate row on any tier change

    if (engine.tier() != RiskTier::DANGER) return;
    if (!wifi.connected()) return;
    if (millis() - lastDiscordMs < DISCORD_COOLDOWN_MS) return;
    lastDiscordMs = millis();

    discord.sendZoneAlert(engine.nearestCm());  // failure is non-fatal
}

static void handleRoot() {
    server.send_P(200, "text/html", WEBUI_PAGE);
}

static void handleStatus() {
    server.send(200, "application/json", buildStatusJson());
}

static void handleToggle() {
    engine.setRunning(!engine.isRunning());
    Logger.printf("[ENGINE] %s\n", engine.isRunning() ? "RUNNING" : "PAUSED");
    server.send(200, "application/json", buildStatusJson());
}

static void handleNotFound() {
    server.send(404, "text/plain", "Not found");
}

void setup() {
    Logger.begin(115200);
    Logger.println("=== MiniProject: Danger Zone Alert ===");

    us1.begin();
    us2.begin();
    engineButton.begin();
    engine.begin(&us1, &us2, &ledSafe, &ledWarn, &ledDanger, &buzzer);

    wifi.begin();
    ota.begin();

    server.on("/", HTTP_GET, handleRoot);
    server.on("/api/status", HTTP_GET, handleStatus);
    server.on("/api/toggle", HTTP_POST, handleToggle);
    server.onNotFound(handleNotFound);
    server.begin();

    if (wifi.connected()) {
        Logger.printf("[WEB] http://%s/\n", wifi.localIP().toString().c_str());
    } else {
        Logger.println("[WEB] WiFi offline — alarm core runs local-only");
    }
}

void loop() {
    if (engineButton.wasPressed()) {
        engine.setRunning(!engine.isRunning());
        Logger.printf("[ENGINE] %s\n", engine.isRunning() ? "RUNNING" : "PAUSED");
    }

    engine.handle();
    alertOnDangerEntry();
    logToSheetsIfNeeded(false);

    Logger.handle();
    ota.handle();
    server.handleClient();
}
