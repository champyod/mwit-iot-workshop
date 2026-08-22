#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Log.h>
#include <OTA.h>
#include <MWIT_WiFi.h>
#include <LED.h>
#include <GG_Sheet.h>
#include <DiscordWebhook.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <string.h>
#include "pins.h"
#include "credentials.h"
#include "webui.h"
#include "UltrasonicRCW.h"
#include "Button.h"
#include "ZoneEngine.h"

MWITWiFi wifi(WIFI_SSID, WIFI_IDENTITY, WIFI_USERNAME, WIFI_PASSWORD);
OTA ota("miniproject-ota");
AsyncWebServer server(80);
static AsyncEventSource events("/api/events");
static String cfgBody;
static volatile bool btnShortReq = false;
static volatile bool btnLongReq = false;
static volatile bool httpToggleReq = false;

static void buttonTask(void *arg);

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
constexpr unsigned long SENSOR_REPORT_INTERVAL_MS = 1000UL;
constexpr unsigned long SENSOR_OK_HEARTBEAT_MS    = 10000UL;
constexpr unsigned long EVENT_PUSH_INTERVAL_MS    = 500UL;
constexpr bool RUN_ECHO_PROBE = false;
constexpr unsigned long PROBE_WINDOW_US = 60000UL;
constexpr unsigned long DEFAULT_ECHO_TIMEOUT_MS = 60UL;
const char* RUN_LABEL = "danger-zone";

// Cloud logging runs on a low-priority FreeRTOS task so TLS connects and
// webhook retries (which can block for many seconds on flaky WiFi) never
// stall loop()'s LED/button/web handling.
enum class LogJobType : uint8_t { SHEET_ROW, DISCORD_ALERT };
struct LogJob {
    LogJobType type;
    float nearestCm;
    char tier[8];
};
constexpr UBaseType_t LOG_QUEUE_LEN = 8;
constexpr UBaseType_t LOG_TASK_PRIORITY = 1;
constexpr BaseType_t LOG_TASK_CORE = 0;      // loop() + web server live on core 1
constexpr uint32_t LOG_TASK_STACK_WORDS = 16384;  // TLS handshake headroom
}

static const char* tierName(RiskTier tier) {
    switch (tier) {
    case RiskTier::DANGER: return "DANGER";
    case RiskTier::WARN:   return "WARN";
    case RiskTier::SAFE:   return "SAFE";
    }
    return "SAFE";
}

static const char* echoStatusName(EchoStatus status) {
    switch (status) {
    case EchoStatus::OK:           return "ok";
    case EchoStatus::NO_ECHO:      return "no-echo";
    case EchoStatus::OUT_OF_RANGE: return "out-of-range";
    }
    return "unknown";
}

// Boot-time wiring check: a dark LED here means pin/wiring fault, not zone logic.
static void selfTestOutput(LED& out, const char* name) {
    Logger.printf("[SELFTEST] %s ON\n", name);
    out.on();
    delay(400);
    out.off();
    delay(150);
}

static QueueHandle_t logQueue = nullptr;

// Consumes queued sheet rows / webhook alerts. Blocking TLS work is safe
// here: only this task waits on it, loop() keeps servicing LEDs, button,
// and the web server.
static void cloudLogTask(void*) {
    LogJob job;
    for (;;) {
        if (xQueueReceive(logQueue, &job, portMAX_DELAY) != pdTRUE) continue;
        switch (job.type) {
        case LogJobType::SHEET_ROW:
            sheets.sendZone(RUN_LABEL, job.nearestCm, job.tier);
            break;
        case LogJobType::DISCORD_ALERT:
            discord.sendZoneAlert(job.nearestCm);
            break;
        }
    }
}

static bool enqueueLogJob(const LogJob& job) {
    if (logQueue == nullptr) return false;
    if (xQueueSend(logQueue, &job, 0) == pdTRUE) return true;
    // Queue full (network stalled) — drop oldest instead of blocking.
    LogJob dropped;
    xQueueReceive(logQueue, &dropped, 0);
    Logger.println("[CLOUD] queue full — dropped oldest job");
    return xQueueSend(logQueue, &job, 0) == pdTRUE;
}

// Hardware bring-up diagnostic: bit-bangs TRIG and polls ECHO with
// digitalRead, bypassing pulseIn. Reports whether the module raises the
// line at all (power/pinout verdict) and the raw pulse geometry when it
// does. Runs once at boot before WiFi; flip RUN_ECHO_PROBE to false to
// disable.
static void echoProbe(uint8_t idx, uint8_t trigPin, uint8_t echoPin) {
    pinMode(trigPin, OUTPUT);
    digitalWrite(trigPin, LOW);
    pinMode(echoPin, INPUT);
    Logger.printf("[PROBE] #%u (trig %u echo %u) idle trig=%d echo=%d\n",
                  idx, trigPin, echoPin, digitalRead(trigPin), digitalRead(echoPin));

    for (uint8_t attempt = 1; attempt <= 3; ++attempt) {
        delay(60);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(12);
        digitalWrite(trigPin, LOW);

        const unsigned long t0 = micros();
        unsigned long riseUs = 0;
        while (micros() - t0 < PROBE_WINDOW_US) {
            if (digitalRead(echoPin) == HIGH) {
                riseUs = micros() - t0;
                break;
            }
        }
        unsigned long highUs = 0;
        if (riseUs > 0) {
            const unsigned long t1 = micros();
            while (digitalRead(echoPin) == HIGH && micros() - t1 < PROBE_WINDOW_US) {}
            highUs = micros() - t1;
        }

        if (riseUs == 0) {
            Logger.printf("[PROBE] #%u ping %u: NO RISE in 60ms — module silent\n", idx, attempt);
        } else {
            Logger.printf("[PROBE] #%u ping %u: rise +%luus, high %luus (~%.1f cm)\n",
                          idx, attempt, riseUs, highUs, highUs / 58.0f);
        }
    }
}

static String buildStatusJson() {
    String json = "{";
    json += "\"uptime_ms\":" + String(millis());
    json += ",\"wifi_connected\":" + String(wifi.connected() ? "true" : "false");
    json += ",\"ssid\":\"" + String(wifi.ssid()) + "\"";
    json += ",\"ip\":\"" + wifi.localIP().toString() + "\"";
    json += ",\"rssi_dbm\":" + String(wifi.rssi());
    json += ",\"free_heap\":" + String(ESP.getFreeHeap());
    json += ",\"running\":" + String(engine.isRunning() ? "true" : "false");
    json += ",\"nearest_cm\":" + String(engine.nearestCm(), 1);
    json += ",\"tier\":\"" + String(tierName(engine.tier())) + "\"";
    json += ",\"danger_cm\":" + String(engine.dangerThresh(), 1);
    json += ",\"warn_cm\":" + String(engine.warnThresh(), 1);
    json += ",\"sample_interval_ms\":" + String(engine.sampleIntervalMs());
    json += ",\"echo_timeout_ms\":" + String(us1.echoTimeoutMs());
    json += ",\"sensors\":[";
    for (uint8_t i = 0; i < engine.sensorCount(); ++i) {
        if (i) json += ",";
        json += "{\"raw\":" + String(engine.sensorRawCm(i), 1);
        json += ",\"cm\":" + String(engine.sensorCm(i), 1);
        json += ",\"offset\":" + String(engine.calibrationOffset(i), 1);
        json += ",\"scale\":" + String(engine.calibrationScale(i), 2);
        json += ",\"delay_ms\":" + String(engine.sensorDelayMs(i));
        json += ",\"enabled\":" + String(engine.isEnabled(i) ? "true" : "false");
        json += ",\"status\":\"" + String(echoStatusName(engine.sensorStatus(i))) + "\"}";
    }
    json += "]";
    json += "}";
    return json;
}

static String buildConfigJson() {
    String json = "{";
    json += "\"danger_cm\":" + String(engine.dangerThresh(), 1);
    json += ",\"warn_cm\":" + String(engine.warnThresh(), 1);
    json += ",\"sample_interval_ms\":" + String(engine.sampleIntervalMs());
    json += ",\"echo_timeout_ms\":" + String(us1.echoTimeoutMs());
    json += ",\"sensors\":[";
    for (uint8_t i = 0; i < engine.sensorCount(); ++i) {
        if (i) json += ",";
        json += "{\"offset\":" + String(engine.calibrationOffset(i), 1);
        json += ",\"scale\":" + String(engine.calibrationScale(i), 2);
        json += ",\"delay_ms\":" + String(engine.sensorDelayMs(i));
        json += ",\"enabled\":" + String(engine.isEnabled(i) ? "true" : "false") + "}";
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

static bool parseJsonValueAt(const String& body, int colon, float& out) {
    int s = colon + 1;
    while (s < (int)body.length() && isspace(body[s])) ++s;
    int e = s;
    while (e < (int)body.length() && (isDigit(body[e]) || body[e]=='.' || body[e]=='-' || body[e]=='+')) ++e;
    if (e <= s) return false;
    out = body.substring(s, e).toFloat();
    return true;
}

static void logToSheetsIfNeeded(bool forceRow) {
    static unsigned long lastSheetMs = 0;
    if (!wifi.connected() || !engine.isRunning()) return;

    const bool due = forceRow || (millis() - lastSheetMs >= SHEET_LOG_INTERVAL_MS);
    if (!due) return;
    lastSheetMs = millis();

    LogJob job{};
    job.type = LogJobType::SHEET_ROW;
    job.nearestCm = engine.nearestCm();
    strncpy(job.tier, tierName(engine.tier()), sizeof(job.tier) - 1);
    job.tier[sizeof(job.tier) - 1] = '\0';
    enqueueLogJob(job);
}

static void pushEvents();

static void alertOnDangerEntry() {
    static unsigned long lastDiscordMs = 0;
    if (!engine.consumeTierChanged()) return;
    pushEvents();

    logToSheetsIfNeeded(true);  // immediate row on any tier change

    if (engine.tier() != RiskTier::DANGER) return;
    if (!wifi.connected()) return;
    if (millis() - lastDiscordMs < DISCORD_COOLDOWN_MS) return;
    lastDiscordMs = millis();

    LogJob job{};
    job.type = LogJobType::DISCORD_ALERT;
    job.nearestCm = engine.nearestCm();
    enqueueLogJob(job);  // failure is non-fatal
}

static void handleRoot(AsyncWebServerRequest *req) {
    req->send_P(200, "text/html", WEBUI_PAGE);
}

static void handleStatus(AsyncWebServerRequest *req) {
    req->send(200, "application/json", buildStatusJson());
}

// Broadcast current state to every connected SSE client. The async server
// keeps each /api/events connection open on its own task, so this is a true
// server push — loop() only has to call it on a cadence or on state changes.
static void pushEvents() {
    events.send(buildStatusJson().c_str(), nullptr, millis());
}

static void handleToggle(AsyncWebServerRequest *req) {
    // Async handlers run on the tcp task; mutating the engine here races with
    // loop()'s sample cycle (applyOutputs can re-light outputs after the stop).
    // Defer like the button flags so the change applies between cycles.
    httpToggleReq = true;
    req->send(200, "application/json", buildStatusJson());
}

static void handleConfigBody(AsyncWebServerRequest *req, uint8_t *data,
                             size_t len, size_t index, size_t total) {
    (void)req;
    if (req->url() != "/api/config") return;
    if (index == 0) cfgBody = "";
    for (size_t i = 0; i < len; ++i) cfgBody += (char)data[i];
}

static void handleConfig(AsyncWebServerRequest *req) {
    const String body = cfgBody;
    cfgBody = "";
    bool any = false;
    float danger = engine.dangerThresh();
    float warn = engine.warnThresh();
    float tmp;
    if (parseJsonFloat(body, "danger_cm", tmp)) { danger = tmp; any = true; }
    if (parseJsonFloat(body, "warn_cm", tmp)) { warn = tmp; any = true; }
    if (any) engine.setThresholds(danger, warn);
    if (parseJsonFloat(body, "sample_interval_ms", tmp)) engine.setSampleIntervalMs((unsigned long)tmp);
    if (parseJsonFloat(body, "echo_timeout_ms", tmp)) {
        us1.setTimeoutUs((unsigned long)(tmp * 1000.0f));
        us2.setTimeoutUs((unsigned long)(tmp * 1000.0f));
    }
    int sensorIdx = 0;
    int searchFrom = 0;
    while (sensorIdx < engine.sensorCount()) {
        const int offPos = body.indexOf("\"offset\"", searchFrom);
        if (offPos < 0) break;
        const int offColon = body.indexOf(':', offPos);
        if (offColon < 0) break;
        const int nextOffPos = body.indexOf("\"offset\"", offPos + 1);
        const int winEnd = (nextOffPos < 0) ? (int)body.length() : nextOffPos;

        float off = engine.calibrationOffset(sensorIdx);
        parseJsonValueAt(body, offColon, off);

        float sc = engine.calibrationScale(sensorIdx);
        int p = body.indexOf("\"scale\"", offPos);
        if (p >= 0 && p < winEnd) {
            const int c = body.indexOf(':', p);
            if (c >= 0) parseJsonValueAt(body, c, sc);
        }

        unsigned long dlyMs = engine.sensorDelayMs(sensorIdx);
        p = body.indexOf("\"delay_ms\"", offPos);
        if (p >= 0 && p < winEnd) {
            const int c = body.indexOf(':', p);
            float v;
            if (c >= 0 && parseJsonValueAt(body, c, v)) dlyMs = (unsigned long)v;
        }

        bool hasEnabled = false;
        bool en = true;
        p = body.indexOf("\"enabled\"", offPos);
        if (p >= 0 && p < winEnd) {
            const int c = body.indexOf(':', p);
            if (c >= 0) {
                const int t = body.indexOf("true", c);
                const int f = body.indexOf("false", c);
                hasEnabled = (t >= 0 || f >= 0);
                en = (t >= 0 && (f < 0 || t < f));
            }
        }

        engine.setCalibration(sensorIdx, off, sc);
        engine.setSensorDelayMs(sensorIdx, dlyMs);
        if (hasEnabled) engine.setEnabled(sensorIdx, en);
        searchFrom = offPos + 1;
        ++sensorIdx;
    }
    Logger.printf("[CONFIG] danger %.1f warn %.1f interval %lums timeout %lums\n",
                  engine.dangerThresh(), engine.warnThresh(),
                  engine.sampleIntervalMs(), us1.echoTimeoutMs());
    pushEvents();
    req->send(200, "application/json", buildConfigJson());
}

static void handleReset(AsyncWebServerRequest *req) {
    engine.resetDefaults();
    us1.setTimeoutUs(DEFAULT_ECHO_TIMEOUT_MS * 1000UL);
    us2.setTimeoutUs(DEFAULT_ECHO_TIMEOUT_MS * 1000UL);
    Logger.println("[ENGINE] RESET defaults");
    pushEvents();
    req->send(200, "application/json", buildConfigJson());
}

static void handleNotFound(AsyncWebServerRequest *req) {
    req->send(404, "text/plain", "Not found");
}

// 1 Hz serial health line: prints while any sensor fails (with reason +
// consecutive-fail count), on every ok/fail state change, and as a 10 s
// all-ok heartbeat. Bounded to one line per second so telnet stays readable.
static void reportSensorHealth() {
    static unsigned long lastReportMs = 0;
    static unsigned long lastOkPrintMs = 0;
    static bool lastOk[2] = {false, false};
    static bool haveLast = false;

    if (!engine.isRunning()) {
        haveLast = false;
        return;
    }
    const unsigned long now = millis();
    if (now - lastReportMs < SENSOR_REPORT_INTERVAL_MS) return;
    lastReportMs = now;

    bool allOk = true;
    bool stateChanged = false;
    char line[128];
    int n = snprintf(line, sizeof(line), "[US]");
    for (uint8_t i = 0; i < engine.sensorCount(); ++i) {
        if (!engine.isEnabled(i)) {
            if (!haveLast || lastOk[i]) stateChanged = true;
            n += snprintf(line + n, sizeof(line) - static_cast<size_t>(n), " #%u OFF", i + 1);
            lastOk[i] = false;
            continue;
        }
        const bool ok = engine.sensorStatus(i) == EchoStatus::OK;
        if (!haveLast || ok != lastOk[i]) stateChanged = true;
        if (ok) {
            n += snprintf(line + n, sizeof(line) - static_cast<size_t>(n),
                          " #%u %.1fcm", i + 1, engine.sensorRawCm(i));
        } else {
            n += snprintf(line + n, sizeof(line) - static_cast<size_t>(n),
                          " #%u FAIL(%s)x%u",
                          i + 1, echoStatusName(engine.sensorStatus(i)),
                          engine.sensorFailStreak(i));
        }
        lastOk[i] = ok;
        allOk = allOk && ok;
    }
    haveLast = true;

    const bool heartbeatDue = allOk && !stateChanged &&
                              (now - lastOkPrintMs >= SENSOR_OK_HEARTBEAT_MS);
    if (allOk && stateChanged) lastOkPrintMs = now;
    if (!allOk || stateChanged || heartbeatDue) Logger.println(line);
}

// Owns the button exclusively and polls fast enough that taps shorter than
// loop()'s sensor-blocking windows are never missed. loop() consumes flags.
static void buttonTask(void *arg) {
    (void)arg;
    for (;;) {
        if (engineButton.wasLongPressed(3000)) btnLongReq = true;
        if (engineButton.wasPressed()) btnShortReq = true;
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void setup() {
    Logger.begin(115200);
    Logger.println("=== MiniProject: Danger Zone Alert ===");

    for (uint8_t p : {PIN_LED_SAFE, PIN_LED_WARN, PIN_LED_DANGER, PIN_BUZZER}) {
        pinMode(p, OUTPUT);
        digitalWrite(p, LOW);
    }

    us1.begin();
    us2.begin();
    if (RUN_ECHO_PROBE) {
        Logger.println("[PROBE] === ultrasonic echo probe ===");
        echoProbe(1, PIN_US1_TRIG, PIN_US1_ECHO);
        delay(30);
        echoProbe(2, PIN_US2_TRIG, PIN_US2_ECHO);
        Logger.println("[PROBE] === end ===");
    }
    engineButton.begin();
    engine.begin(&us1, &us2, &ledSafe, &ledWarn, &ledDanger, &buzzer);

    selfTestOutput(ledSafe,   "SAFE   LED GPIO26");
    selfTestOutput(ledWarn,   "WARN   LED GPIO27");
    selfTestOutput(ledDanger, "DANGER LED GPIO25");
    selfTestOutput(buzzer,    "BUZZER     GPIO14");

    wifi.begin();
    ota.begin();

    if (xTaskCreatePinnedToCore(buttonTask, "button", 2048, nullptr,
                                2, nullptr, 0) != pdPASS) {
        Logger.println("[BTN] failed to start button task");
    }

    server.on("/", HTTP_GET, handleRoot);
    server.on("/api/status", HTTP_GET, handleStatus);
    server.addHandler(&events);
    server.on("/api/toggle", HTTP_POST, handleToggle);
    server.on("/api/config", HTTP_POST, handleConfig, NULL, handleConfigBody);
    server.on("/api/reset", HTTP_POST, handleReset);
    server.onNotFound(handleNotFound);
    server.begin();

    logQueue = xQueueCreate(LOG_QUEUE_LEN, sizeof(LogJob));
    if (logQueue != nullptr) {
        if (xTaskCreatePinnedToCore(cloudLogTask, "cloud-log",
                                    LOG_TASK_STACK_WORDS, nullptr,
                                    LOG_TASK_PRIORITY, nullptr,
                                    LOG_TASK_CORE) != pdPASS) {
            Logger.println("[CLOUD] failed to start log task");
        }
    } else {
        Logger.println("[CLOUD] failed to create log queue");
    }

    if (wifi.connected()) {
        Logger.printf("[WEB] http://%s/\n", wifi.localIP().toString().c_str());
    } else {
        Logger.println("[WEB] WiFi offline — alarm core runs local-only");
    }
}

void loop() {
    if (btnLongReq) {
        btnLongReq = false;
        engine.resetDefaults();
        us1.setTimeoutUs(DEFAULT_ECHO_TIMEOUT_MS * 1000UL);
        us2.setTimeoutUs(DEFAULT_ECHO_TIMEOUT_MS * 1000UL);
        Logger.println("[ENGINE] RESET defaults (long press)");
        pushEvents();
    } else if (btnShortReq) {
        btnShortReq = false;
        engine.setRunning(!engine.isRunning());
        Logger.printf("[ENGINE] %s\n", engine.isRunning() ? "RUNNING" : "PAUSED");
        pushEvents();
    } else if (httpToggleReq) {
        httpToggleReq = false;
        engine.setRunning(!engine.isRunning());
        Logger.printf("[ENGINE] %s (http)\n", engine.isRunning() ? "RUNNING" : "PAUSED");
        pushEvents();
    }

    engine.handle();
    alertOnDangerEntry();
    reportSensorHealth();
    logToSheetsIfNeeded(false);

    Logger.handle();
    ota.handle();

    static unsigned long lastEventPushMs = 0;
    const unsigned long now = millis();
    if (now - lastEventPushMs >= EVENT_PUSH_INTERVAL_MS) {
        lastEventPushMs = now;
        pushEvents();
    }
}
