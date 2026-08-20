#include "OTA.h"
#include <Log.h>
#include <Update.h>

OTA* OTA::instance_ = nullptr;

OTA::OTA(const char* hostname, const char* password, uint16_t port)
    : hostname_(hostname), password_(password), port_(port)
    , server_(nullptr), state_(IDLE)
    , expectedSize_(0), received_(0), lastPacket_(0), serverReady_(false) {
    instance_ = this;
}

void OTA::begin() {
    WiFi.onEvent(onWiFiEvent);

    if (WiFi.status() == WL_CONNECTED) {
        startServer();
    }
}

void OTA::startServer() {
    if (serverReady_) return;
    server_ = new WiFiServer(port_);
    server_->begin();
    serverReady_ = true;
    Logger.printf("[OTA] Ready on port %d — upload: ota_upload.py firmware.bin %s\n",
                  port_, WiFi.localIP().toString().c_str());
}

void OTA::stopServer() {
    if (client_) {
        client_.stop();
        Update.abort();
    }
    if (server_) {
        server_->stop();
        delete server_;
        server_ = nullptr;
    }
    serverReady_ = false;
    state_ = IDLE;
    expectedSize_ = 0;
    received_ = 0;
}

void OTA::handle() {
    unsigned long now = millis();

    // Timeout: 30s no data while uploading
    if (state_ == UPLOAD && now - lastPacket_ > 30000) {
        Logger.println("[OTA] Timeout — no data for 30s");
        fail(1);
        return;
    }

    if (state_ == DONE) {
        Logger.println("[OTA] Success — rebooting...\n");
        Logger.closeAll();  // FIN to monitors; nc exits cleanly instead of hanging
        delay(500);
        ESP.restart();
        return;
    }

    if (!serverReady_) return;

    // Accept new client when idle
    if (state_ == IDLE) {
        client_ = server_->available();
        if (client_) {
            state_ = AUTH;
            expectedSize_ = 0;
            received_ = 0;
            Logger.println("[OTA] Client connected");
        }
        return;
    }

    if (!client_ || !client_.connected()) {
        if (state_ != IDLE) {
            Logger.println("[OTA] Client disconnected unexpectedly");
            state_ = IDLE;
        }
        return;
    }

    // AUTH phase: read 8-byte header + password
    if (state_ == AUTH) {
        if (client_.available() < 8) return;

        uint8_t hdr[8];
        client_.read(hdr, 8);
        uint32_t cmd = hdr[0] | (hdr[1] << 8) | (hdr[2] << 16) | (hdr[3] << 24);
        expectedSize_ = hdr[4] | (hdr[5] << 8) | (hdr[6] << 16) | (hdr[7] << 24);

        // Read null-terminated password
        String clientPass;
        while (true) {
            if (!client_.available()) { delay(10); continue; }
            char c = client_.read();
            if (c == 0) break;
            clientPass += c;
        }

        // Verify password (if set)
        if (password_ && clientPass != password_) {
            Logger.println("[OTA] Auth failed");
            client_.print("AUTH FAIL");
            delay(100);
            client_.stop();
            state_ = IDLE;
            return;
        }

        if (expectedSize_ == 0 || expectedSize_ > 0x200000) {
            Logger.printf("[OTA] Invalid size: %u\n", (unsigned)expectedSize_);
            fail(2);
            return;
        }

        if (!Update.begin(expectedSize_)) {
            Logger.printf("[OTA] Update.begin failed: %s\n", Update.errorString());
            fail(3);
            return;
        }

        state_ = UPLOAD;
        lastPacket_ = millis();
        if (onStart_) onStart_();
        Logger.printf("[OTA] Receiving %u bytes...\n", (unsigned)expectedSize_);
        return;
    }

    // UPLOAD phase: stream data
    if (state_ == UPLOAD) {
        size_t avail = client_.available();
        if (avail == 0) return;

        uint8_t buf[4096];
        while (client_.available()) {
            int toRead = min((int)sizeof(buf), (int)client_.available());
            size_t r = client_.read(buf, toRead);
            size_t w = Update.write(buf, r);
            if (w != r) {
                Logger.printf("[OTA] Write error: %s\n", Update.errorString());
                fail(4);
                return;
            }
            received_ += r;
        }
        lastPacket_ = millis();
        if (onProgress_) onProgress_(received_, expectedSize_);
    }

    // Check if upload complete
    if (state_ == UPLOAD && received_ >= expectedSize_) {
        if (!Update.end(true)) {
            Logger.printf("[OTA] Update.end failed: %s\n", Update.errorString());
            fail(5);
            return;
        }
        if (onEnd_) onEnd_();
        client_.print("OK");
        client_.flush();
        delay(100);
        client_.stop();
        state_ = DONE;
    }
}

void OTA::fail(int err) {
    if (onError_) onError_(err);
    if (client_) {
        client_.print("ERR");
        client_.flush();
        delay(100);
        client_.stop();
    }
    Update.abort();
    state_ = IDLE;
}

void OTA::onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
    if (!instance_) return;

    if (event == WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP) {
        Logger.printf("[OTA] WiFi ready (%s)\n",
                      WiFi.localIP().toString().c_str());
        instance_->startServer();
        return;
    }

    if (event == WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
        Logger.println("[OTA] WiFi lost — stopping server");
        instance_->stopServer();
    }
}
