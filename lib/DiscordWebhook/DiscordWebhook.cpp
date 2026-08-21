#include "DiscordWebhook.h"
#include <Log.h>
#include <time.h>

// GTS Root R1 — same CA as GG_Sheet. Discord API chains through this.
const char* DiscordWebhook::ROOT_CA =
"-----BEGIN CERTIFICATE-----\n"
"MIIFVzCCAz+gAwIBAgINAgPlk28xsBNJiGuiFzANBgkqhkiG9w0BAQwFADBHMQsw\n"
"CQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEU\n"
"MBIGA1UEAxMLR1RTIFJvb3QgUjEwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAw\n"
"MDAwWjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZp\n"
"Y2VzIExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjEwggIiMA0GCSqGSIb3DQEBAQUA\n"
"A4ICDwAwggIKAoICAQC2EQKLHuOhd5s73L+UPreVp0A8of2C+X0yBoJx9vaMf/vo\n"
"27xqLpeXo4xL+Sv2sfnOhB2x+cWX3u+58qPpvBKJXqeqUqv4IyfLpLGcY9vXmX7w\n"
"Cl7raKb0xlpHDU0QM+NOsROjyBhsS+z8CZDfnWQpJSMHobTSPS5g4M/SCYe7zUjw\n"
"TcLCeoiKu7rPWRnWr4+wB7CeMfGCwcDfLqZtbBkOtdh+JhpFAz2weaSUKK0Pfybl\n"
"qAj+lug8aJRT7oM6iCsVlgmy4HqMLnXWnOunVmSPlk9orj2XwoSPwLxAwAtcvfaH\n"
"szVsrBhQf4TgTM2S0yDpM7xSma8ytSmzJSq0SPly4cpk9+aCEI3oncKKiPo4Zor8\n"
"Y/kB+Xj9e1x3+naH+uzfsQ55lVe0vSbv1gHR6xYKu44LtcXFilWr06zqkUspzBmk\n"
"MiVOKvFlRNACzqrOSbTqn3yDsEB750Orp2yjj32JgfpMpf/VjsPOS+C12LOORc92\n"
"wO1AK/1TD7Cn1TsNsYqiA94xrcx36m97PtbfkSIS5r762DL8EGMUUXLeXdYWk70p\n"
"aDPvOmbsB4om3xPXV2V4J95eSRQAogB/mqghtqmxlbCluQ0WEdrHbEg8QOB+DVrN\n"
"VjzRlwW5y0vtOUucxD/SVRNuJLDWcfr0wbrM7Rv1/oFB2ACYPTrIrnqYNxgFlQID\n"
"AQABo0IwQDAOBgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4E\n"
"FgQU5K8rJnEaK0gnhS9SZizv8IkTcT4wDQYJKoZIhvcNAQEMBQADggIBAJ+qQibb\n"
"C5u+/x6Wki4+omVKapi6Ist9wTrYggoGxval3sBOh2Z5ofmmWJyq+bXmYOfg6LEe\n"
"QkEzCzc9zolwFcq1JKjPa7XSQCGYzyI0zzvFIoTgxQ6KfF2I5DUkzps+GlQebtuy\n"
"h6f88/qBVRRiClmpIgUxPoLW7ttXNLwzldMXG+gnoot7TiYaelpkttGsN/H9oPM4\n"
"7HLwEXWdyzRSjeZ2axfG34arJ45JK3VmgRAhpuo+9K4l/3wV3s6MJT/KYnAK9y8J\n"
"ZgfIPxz88NtFMN9iiMG1D53Dn0reWVlHxYciNuaCp+0KueIHoI17eko8cdLiA6Ef\n"
"MgfdG+RCzgwARWGAtQsgWSl4vflVy2PFPEz0tv/bal8xa5meLMFrUKTX5hgUvYU/\n"
"Z6tGn6D/Qqc6f1zLXbBwHSs09dR2CQzreExZBfMzQsNhFRAbd03OIozUhfJFfbdT\n"
"6u9AWpQKXCBfTkBdYiJ23//OYb2MI3jSNwLgjt7RETeJ9r/tSQdirpLsQBqvFAnZ\n"
"0E6yove+7u7Y/9waLd64NnHi/Hm3lCXRSHNboTXns5lndcEZOitHTtNCjv0xyBZm\n"
"2tIMPNuzjsmhDYAPexZ3FL//2wmUspO8IFgV6dtxQ/PeEMMA3KgqlbbC1j+Qa3bb\n"
"bP6MvPJwNQzcmRk13NfIRmPVNnGuV/u3gm3c\n"
"-----END CERTIFICATE-----\n";

DiscordWebhook::DiscordWebhook(const char* url) {
    parseUrl(url);
}

bool DiscordWebhook::parseUrl(const char* url) {
    String s(url);
    int start = s.indexOf("://");
    if (start < 0) return false;
    start += 3;

    int slash = s.indexOf('/', start);
    if (slash < 0) {
        host_ = s.substring(start);
        path_ = "/";
    } else {
        host_ = s.substring(start, slash);
        path_ = s.substring(slash);
    }
    return true;
}

String DiscordWebhook::buildTimestamp() {
    time_t now = time(nullptr);
    struct tm* t = gmtime(&now);
    char buf[25];
    snprintf(buf, sizeof(buf),
             "%04d-%02d-%02dT%02d:%02d:%02d.000Z",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);
    return String(buf);
}

String DiscordWebhook::buildAlertPayload(float temperature, float humidity, float heatIndex) {
    String ts = buildTimestamp();
    // 350 byte reserve is generous for this payload
    String json;
    json.reserve(450);

    json += "{\"embeds\":[{\"color\":";
    json += 15158332;  // red
    json += ",\"title\":\"Server Room Alert\"";
    json += ",\"description\":\"Temperature or humidity exceeded safe threshold.\"";
    json += ",\"timestamp\":\"";
    json += ts;
    json += "\",\"fields\":[";

    json += "{\"name\":\"Temperature\",\"value\":\"";
    json += String(temperature, 1);
    json += " C\",\"inline\":true},";

    json += "{\"name\":\"Humidity\",\"value\":\"";
    json += String(humidity, 1);
    json += " %\",\"inline\":true},";

    json += "{\"name\":\"Heat Index\",\"value\":\"";
    json += String(heatIndex, 1);
    json += " C\",\"inline\":true},";

    json += "{\"name\":\"Threshold\",\"value\":\">30 C  or  >80% RH\",\"inline\":false},";

    json += "{\"name\":\"Status\",\"value\":\"ALERT\",\"inline\":true}";

    json += "],\"footer\":{\"text\":\"activity7\"}";
    json += "}]}";

    return json;
}

String DiscordWebhook::buildRecoveryPayload(float temperature, float humidity, float heatIndex) {
    String ts = buildTimestamp();
    String json;
    json.reserve(400);

    json += "{\"embeds\":[{\"color\":";
    json += 5763719;  // green
    json += ",\"title\":\"Server Room Recovered\"";
    json += ",\"description\":\"All values returned within safe range.\"";
    json += ",\"timestamp\":\"";
    json += ts;
    json += "\",\"fields\":[";

    json += "{\"name\":\"Temperature\",\"value\":\"";
    json += String(temperature, 1);
    json += " C\",\"inline\":true},";

    json += "{\"name\":\"Humidity\",\"value\":\"";
    json += String(humidity, 1);
    json += " %\",\"inline\":true},";

    json += "{\"name\":\"Heat Index\",\"value\":\"";
    json += String(heatIndex, 1);
    json += " C\",\"inline\":true},";

    json += "{\"name\":\"Status\",\"value\":\"NORMAL\",\"inline\":true}";

    json += "],\"footer\":{\"text\":\"activity7\"}";
    json += "}]}";

    return json;
}

bool DiscordWebhook::sendPayload(const String& payload) {
    Logger.printf("[Discord] POST %s%s (%d bytes)\n",
                  host_.c_str(), path_.c_str(), payload.length());

    for (int attempt = 0; attempt < 2; attempt++) {
        client_.setCACert(ROOT_CA);
        client_.setTimeout(8000);

        // Check WiFi before trying
        if (WiFi.status() != WL_CONNECTED) {
            Logger.printf("[Discord] WiFi disconnected (status %d)\n", WiFi.status());
            delay(2000);
            continue;
        }
        Logger.printf("[Discord] WiFi RSSI: %d dBm\n", WiFi.RSSI());

        // TCP + TLS connect with hostname for SNI
        Logger.printf("[Discord] Connecting %s:443...\n", host_.c_str());
        if (!client_.connect(host_.c_str(), 443)) {
            delay(2000);
            continue;
        }
        Logger.printf("[Discord] Connected, sending request...\n");

        client_.print("POST ");
        client_.print(path_);
        client_.println(" HTTP/1.1");
        client_.print("Host: ");
        client_.println(host_);
        client_.println("Content-Type: application/json");
        client_.print("Content-Length: ");
        client_.println(payload.length());
        client_.println("Connection: close");
        client_.println();
        client_.print(payload);

        unsigned long start = millis();
        int statusCode = 0;
        int responseLines = 0;
        bool headerEnd = false;
        String responseBody;

        while (client_.connected() && millis() - start < 8000) {
            if (client_.available()) {
                String line = client_.readStringUntil('\n');
                line.trim();
                if (!headerEnd) {
                    if (line == "") {
                        headerEnd = true;
                        Logger.printf("[Discord] Response headers: %d lines\n", responseLines);
                    } else {
                        responseLines++;
                        if (line.startsWith("HTTP/")) {
                            int sp = line.indexOf(' ');
                            if (sp > 0)
                                statusCode = line.substring(sp + 1, sp + 4).toInt();
                            Logger.printf("[Discord] %s\n", line.c_str());
                        } else if (line.startsWith("content-length:", false) ||
                                   line.startsWith("x-ratelimit-", false)) {
                            Logger.printf("[Discord] %s\n", line.c_str());
                        } else if (line.startsWith("retry-after:", false)) {
                            Logger.printf("[Discord] %s\n", line.c_str());
                        }
                    }
                } else {
                    responseBody += line;
                }
            }
        }

        client_.stop();

        bool ok = (statusCode >= 200 && statusCode < 300);
        if (ok) {
            Logger.printf("[Discord] Status: %d (OK)\n", statusCode);
        } else if (statusCode == 429) {
            Logger.printf("[Discord] Status: %d RATE LIMITED — %s\n",
                          statusCode, responseBody.c_str());
        } else if (statusCode == 400) {
            String snippet = responseBody.length() > 120
                ? responseBody.substring(0, 120) + "..."
                : responseBody;
            Logger.printf("[Discord] Status: %d BAD REQUEST — %s\n",
                          statusCode, snippet.c_str());
        } else if (statusCode > 0) {
            Logger.printf("[Discord] Status: %d %s\n",
                          statusCode, responseBody.c_str());
        } else {
            Logger.printf("[Discord] No HTTP response received\n");
        }

        return ok;
    }

    Logger.println("[Discord] Retries exhausted — giving up");
    return false;
}

bool DiscordWebhook::sendAlert(float temperature, float humidity, float heatIndex) {
    String payload = buildAlertPayload(temperature, humidity, heatIndex);
    return sendPayload(payload);
}

bool DiscordWebhook::sendRecovery(float temperature, float humidity, float heatIndex) {
    String payload = buildRecoveryPayload(temperature, humidity, heatIndex);
    return sendPayload(payload);
}

String DiscordWebhook::buildZoneAlertPayload(float nearestCm) {
    const String ts = buildTimestamp();
    String json;
    json.reserve(400);

    json += "{\"embeds\":[{\"color\":";
    json += 15158332;  // red
    json += ",\"title\":\"Danger Zone Alert\"";
    json += ",\"description\":\"Worker entered machinery danger zone.\"";
    json += ",\"timestamp\":\"";
    json += ts;
    json += "\",\"fields\":[";

    json += "{\"name\":\"Nearest Distance\",\"value\":\"";
    json += String(nearestCm, 1);
    json += " cm\",\"inline\":true},";

    json += "{\"name\":\"Threshold\",\"value\":\"<= 50 cm\",\"inline\":true},";

    json += "{\"name\":\"Status\",\"value\":\"DANGER\",\"inline\":true}";

    json += "],\"footer\":{\"text\":\"miniproject\"}";
    json += "}]}";

    return json;
}

bool DiscordWebhook::sendZoneAlert(float nearestCm) {
    String payload = buildZoneAlertPayload(nearestCm);
    return sendPayload(payload);
}
