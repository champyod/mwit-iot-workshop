#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <esp_wpa2.h>

class MWITWiFi {
public:
    MWITWiFi(const char* ssid,
             const char* eap_identity,
             const char* eap_username,
             const char* eap_password);

    void begin();
    bool connected();
    IPAddress localIP();
    int8_t rssi();

private:
    const char* ssid_;
    const char* eap_identity_;
    const char* eap_username_;
    const char* eap_password_;

    volatile bool gotIP_ = false;

    static MWITWiFi* self_;
    static void onGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
    static void onDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
};