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
    json += ",\"danger_cm\":" + String(engine.dangerThresh(), 1);
    json += ",\"warn_cm\":" + String(engine.warnThresh(), 1);
    json += ",\"sensors\":[";
    for (uint8_t i = 0; i < engine.sensorCount(); ++i) {
        if (i) json += ",";
        json += "{\"raw\":" + String(engine.sensorRawCm(i), 1);
        json += ",\"cm\":" + String(engine.sensorCm(i), 1);
        json += ",\"offset\":" + String(engine.calibrationOffset(i), 1);
        json += ",\"scale\":" + String(engine.calibrationScale(i), 2) + "}";
    }
    json += "]";
    json += "}";
    return json;
}

static bool parseJsonFloat(const String& body, const char* key, float& out) {
    String needle = String("\"") + key + "\"";
    int idx = body.indexOf(needle);
    if (idx < 0) return false;
    int colon = body.indexOf(':', idx);
    if (colon < 0) return false;
    int start = colon + 1;
    while (start < (int)body.length() && isspace(body[start])) ++start;
    int end = start;
    while (end < (int)body.length() && (isDigit(body[end]) || body[end]=='.' || body[end]=='-' || body[end]=='+')) ++end;
    if (end <= start) return false;
    out = body.substring(start, end).toFloat();
    return true;
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

static void handleConfig() {
    String body = server.arg("plain");
    bool any = false;
    float danger = engine.dangerThresh();
    float warn = engine.warnThresh();
    float tmp;
    if (parseJsonFloat(body, "danger_cm", tmp)) { danger = tmp; any = true; }
    if (parseJsonFloat(body, "warn_cm", tmp)) { warn = tmp; any = true; }
    if (any) engine.setThresholds(danger, warn);
    int offsetIdx = 0;
    int searchFrom = 0;
    while (offsetIdx < engine.sensorCount()) {
        int offPos = body.indexOf("\"offset\"", searchFrom);
        if (offPos < 0) break;
        int colon = body.indexOf(':', offPos);
        if (colon < 0) break;
        int s = colon + 1;
        while (s < (int)body.length() && isspace(body[s])) ++s;
        int e = s;
        while (e < (int)body.length() && (isDigit(body[e]) || body[e]=='.' || body[e]=='-' || body[e]=='+')) ++e;
        float off = body.substring(s, e).toFloat();
        int scalePos = body.indexOf("\"scale\"", e);
        float sc = engine.calibrationScale(offsetIdx);
        if (scalePos >= 0) {
            int c2 = body.indexOf(':', scalePos);
            if (c2 >= 0) {
                int s2 = c2 + 1;
                while (s2 < (int)body.length() && isspace(body[s2])) ++s2;
                int e2 = s2;
                while (e2 < (int)body.length() && (isDigit(body[e2]) || body[e2]=='.' || body[e2]=='-' || body[e2]=='+')) ++e2;
                if (e2 > s2) sc = body.substring(s2, e2).toFloat();
            }
        }
        engine.setCalibration(offsetIdx, off, sc);
        searchFrom = e + 1;
        offsetIdx++;
    }
    Logger.printf("[CONFIG] danger %.1f warn %.1f\n", engine.dangerThresh(), engine.warnThresh());
    server.send(200, "application/json", buildStatusJson());
}

static void handleReset() {
    engine.resetDefaults();
    Logger.println("[ENGINE] RESET defaults");
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
    server.on("/api/config", HTTP_POST, handleConfig);
    server.on("/api/reset", HTTP_POST, handleReset);
    server.onNotFound(handleNotFound);
    server.begin();

    if (wifi.connected()) {
        Logger.printf("[WEB] http://%s/\n", wifi.localIP().toString().c_str());
    } else {
        Logger.println("[WEB] WiFi offline — alarm core runs local-only");
    }
}

void loop() {
    if (engineButton.wasLongPressed(3000)) {
        engine.resetDefaults();
        Logger.println("[ENGINE] RESET defaults (long press)");
    } else if (engineButton.wasPressed()) {
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
