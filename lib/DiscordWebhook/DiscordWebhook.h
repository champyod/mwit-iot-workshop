#pragma once
#include <Arduino.h>
#include <WiFiClientSecure.h>

class DiscordWebhook {
public:
    explicit DiscordWebhook(const char* url);

    bool sendAlert(float temperature, float humidity, float heatIndex);
    bool sendRecovery(float temperature, float humidity, float heatIndex);
    bool sendZoneAlert(float nearestCm);

private:
    String host_;
    String path_;
    WiFiClientSecure client_;

    static const char* ROOT_CA;

    bool parseUrl(const char* url);
    bool sendPayload(const String& payload);
    String buildTimestamp();
    String buildAlertPayload(float temperature, float humidity, float heatIndex);
    String buildRecoveryPayload(float temperature, float humidity, float heatIndex);
    String buildZoneAlertPayload(float nearestCm);
};
