#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <functional>

class OTA {
public:
    OTA(const char* hostname = "esp32-ota",
        const char* password = nullptr,
        uint16_t port = 3232);

    void begin();
    void handle();

    void onStart(std::function<void()> cb) { onStart_ = cb; }
    void onEnd(std::function<void()> cb) { onEnd_ = cb; }
    void onProgress(std::function<void(unsigned int, unsigned int)> cb) { onProgress_ = cb; }
    void onError(std::function<void(int)> cb) { onError_ = cb; }

private:
    enum State { IDLE, AUTH, UPLOAD, DONE };

    const char* hostname_;
    const char* password_;
    uint16_t port_;

    WiFiServer* server_;
    WiFiClient client_;
    State state_;
    size_t expectedSize_;
    size_t received_;
    unsigned long lastPacket_;
    bool serverReady_;

    std::function<void()> onStart_ = nullptr;
    std::function<void()> onEnd_ = nullptr;
    std::function<void(unsigned int, unsigned int)> onProgress_ = nullptr;
    std::function<void(int)> onError_ = nullptr;

    static OTA* instance_;
    static void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info);
    void startServer();
    void stopServer();
    void fail(int err);
};