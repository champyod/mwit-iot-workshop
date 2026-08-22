#pragma once
#include <Arduino.h>
#include <WiFiClientSecure.h>

// Snapshot of one detection cycle, captured in loop() and pushed by the
// cloud task. Column order mirrors the 'Log' tab in apps-script/Code.gs.
struct ZoneTelemetry {
    const char* run;
    bool        running;
    bool        tierChange;
    float       nearestCm;
    const char* tier;
    float       raw[2];
    float       cm[2];
    const char* status[2];
    int         rssiDbm;
    uint32_t    freeHeap;
    unsigned long uptimeMs;
};

class GGSheet {
public:
    GGSheet(const char* host, const char* path);

    // Legacy DHT22 payload kept for the workshop activities branch.
    bool send(const char* run,
              float temperature,
              float humidity,
              float heatIndex);

    bool sendTelemetry(const ZoneTelemetry& t);

private:
    const char* host_;
    const char* path_;

    String buildQuery(const char* run,
                      float temperature,
                      float humidity,
                      float heatIndex);

    String buildTelemetryQuery(const ZoneTelemetry& t);

    // Shared HTTPS GET with 302-follow and retry loop. Mutates host/path on
    // redirect. Returns true on any 2xx final status.
    bool fetchWithRedirects(String& host, String& path);
};
