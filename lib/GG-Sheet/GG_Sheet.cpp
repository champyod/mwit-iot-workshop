#include "GG_Sheet.h"

// GTS Root R1 — Google Trust Services root CA.
// script.googleusercontent.com chains through GTS CA 1C3 → GTS Root R1.
// Source: https://pki.goog/repo/certs/gtsr1.pem
static const char* ggCaCert = \
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

GGSheet::GGSheet(const char* host, const char* path)
    : host_(host), path_(path) {}

String GGSheet::buildQuery(const char* run,
                           float temperature,
                           float humidity,
                           float heatIndex) {
    return String("?run=")        + run
         + "&temperature=" + temperature
         + "&humidity="    + humidity
         + "&heatIndex="   + heatIndex;
}

String GGSheet::buildTelemetryQuery(const ZoneTelemetry& t) {
    char buf[320];
    snprintf(buf, sizeof(buf),
             "?run=%s&event=%s&running=%s&nearest_cm=%.1f&tier=%s"
             "&s1_raw=%.1f&s1_cm=%.1f&s1_status=%s"
             "&s2_raw=%.1f&s2_cm=%.1f&s2_status=%s"
             "&rssi_dbm=%d&free_heap=%u&uptime_ms=%lu",
             t.run,
             t.tierChange ? "tier_change" : "heartbeat",
             t.running ? "true" : "false",
             t.nearestCm, t.tier,
             t.raw[0], t.cm[0], t.status[0],
             t.raw[1], t.cm[1], t.status[1],
             t.rssiDbm,
             (unsigned)t.freeHeap,
             t.uptimeMs);
    return String(buf);
}

// Parse "Location: https://host/path?..." → host + path+query
static bool parseLocation(const String& line,
                          String& outHost, String& outPath) {
    int colon = line.indexOf(':');
    if (colon < 0) return false;
    String val = line.substring(colon + 1);
    val.trim();

    if (!val.startsWith("https://")) return false;

    val = val.substring(8);  // strip https://
    int slash = val.indexOf('/');
    if (slash < 0) return false;

    outHost = val.substring(0, slash);
    outPath = val.substring(slash);
    return true;
}

bool GGSheet::fetchWithRedirects(String& host, String& path) {
    for (int attempt = 0; attempt < 3; attempt++) {
        WiFiClientSecure client;
        client.setCACert(ggCaCert);
        if (!client.connect(host.c_str(), 443)) {
            Serial.printf("[GGSheet] Connection failed (attempt %d)\n", attempt + 1);
            delay(1000 * (attempt + 1));
            continue;
        }

        client.print("GET ");
        client.print(path);
        client.println(" HTTP/1.1");
        client.print("Host: ");
        client.println(host);
        client.println("Connection: close");
        client.println();

        unsigned long start = millis();
        String responseBody;
        String location;
        bool headerEnd = false;
        int statusCode = 0;

        while (client.connected() && millis() - start < 5000) {
            if (client.available()) {
                String line = client.readStringUntil('\n');
                line.trim();
                if (!headerEnd) {
                    if (line == "") {
                        headerEnd = true;
                    } else if (line.startsWith("HTTP/")) {
                        int sp = line.indexOf(' ');
                        if (sp > 0)
                            statusCode = line.substring(sp + 1, sp + 4).toInt();
                    } else if (line.startsWith("Location:", false)) {
                        location = line;
                    }
                } else {
                    responseBody += line;
                }
            }
        }

        client.stop();

        // 302 → follow redirect
        if (statusCode == 302 && location.length() > 0) {
            String newHost, newPath;
            if (parseLocation(location, newHost, newPath)) {
                Serial.printf("[GGSheet] Follow redirect to %s%s\n",
                              newHost.c_str(), newPath.c_str());
                host = newHost;
                path = newPath;
                continue;
            }
        }

        // Not a redirect or parse failed → final result
        bool ok = (statusCode >= 200 && statusCode < 300);
        if (ok) {
            Serial.printf("[GGSheet] %d: %s\n", statusCode, responseBody.c_str());
        } else {
            Serial.printf("[GGSheet] Failed (status %d)\n", statusCode);
        }
        return ok;
    }

    Serial.println("[GGSheet] Too many redirects or retries exhausted");
    return false;
}

bool GGSheet::send(const char* run,
                   float temperature,
                   float humidity,
                   float heatIndex) {
    String host = host_;
    String path = String(path_) + buildQuery(run, temperature, humidity, heatIndex);
    return fetchWithRedirects(host, path);
}

bool GGSheet::sendTelemetry(const ZoneTelemetry& t) {
    String host = host_;
    String path = String(path_) + buildTelemetryQuery(t);
    return fetchWithRedirects(host, path);
}
