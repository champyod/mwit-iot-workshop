#pragma once
#include <Arduino.h>
#include <WiFiClientSecure.h>

class GGSheet {
public:
    GGSheet(const char* host, const char* path);

    bool send(const char* run,
              float temperature,
              float humidity,
              float heatIndex);

    bool sendZone(const char* run,
                  float nearestCm,
                  const char* tier);

private:
    const char* host_;
    const char* path_;

    String buildQuery(const char* run,
                      float temperature,
                      float humidity,
                      float heatIndex);

    String buildZoneQuery(const char* run,
                          float nearestCm,
                          const char* tier);

    // Shared HTTPS GET with 302-follow and retry loop. Mutates host/path on
    // redirect. Returns true on any 2xx final status.
    bool fetchWithRedirects(String& host, String& path);
};
