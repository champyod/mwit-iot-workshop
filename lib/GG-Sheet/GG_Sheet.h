#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

class GGSheet {
public:
    GGSheet(const char* host, const char* path);

    bool send(const char* run,
              float temperature,
              float humidity,
              float heatIndex);

private:
    const char* host_;
    const char* path_;

    String buildQuery(const char* run,
                      float temperature,
                      float humidity,
                      float heatIndex);
};
