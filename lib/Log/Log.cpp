#include "Log.h"
#include <stdarg.h>

Log Logger;

Log::Log()
    : server_(nullptr), numClients_(0), telnetReady_(false)
    , head_(0), tail_(0), overflow_(false) {}

void Log::begin(int baudRate) {
    Serial.begin(baudRate);
    delay(500);
}

void Log::handle() {
    if (WiFi.status() == WL_CONNECTED && !telnetReady_) {
        startTelnet();
    }

    if (!server_) return;

    WiFiClient newClient = server_->available();
    if (newClient) {
        if (numClients_ < LOG_TELNET_MAX) {
            clients_[numClients_++] = newClient;
            drainOne(newClient);
            if (overflow_) {
                newClient.println("[Log] ... buffer overflow — some messages lost");
                overflow_ = false;
            }
        } else {
            newClient.println("[Log] Max telnet clients reached");
            newClient.stop();
        }
    }

    for (int i = numClients_ - 1; i >= 0; i--) {
        if (!clients_[i].connected()) {
            clients_[i] = clients_[--numClients_];
        }
    }
}

void Log::startTelnet() {
    server_ = new WiFiServer(2323);
    server_->begin();
    telnetReady_ = true;
}

void Log::print(const char* s) {
    size_t len = strlen(s);
    Serial.write((const uint8_t*)s, len);
    toAll(s, len);
}

void Log::println(const char* s) {
    size_t len = strlen(s);
    Serial.write((const uint8_t*)s, len);
    Serial.write('\n');
    toAll(s, len);
    toAll("\r\n", 2);
}

void Log::printf(const char* fmt, ...) {
    char tmp[256];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(tmp, sizeof(tmp), fmt, args);
    va_end(args);
    if (n > 0) {
        size_t len = (size_t)n < sizeof(tmp) ? (size_t)n : sizeof(tmp) - 1;
        Serial.write((const uint8_t*)tmp, len);
        toAll(tmp, len);
    }
}

void Log::print(int n) {
    char tmp[16];
    int len = snprintf(tmp, sizeof(tmp), "%d", n);
    Serial.write((const uint8_t*)tmp, (size_t)len);
    toAll(tmp, (size_t)len);
}

void Log::println(int n) {
    char tmp[16];
    int len = snprintf(tmp, sizeof(tmp), "%d\r\n", n);
    Serial.print(n);
    Serial.println();
    toAll(tmp, (size_t)len);
}

void Log::print(float f, int d) {
    char fmt[8];
    snprintf(fmt, sizeof(fmt), "%%.%df", d);
    char tmp[32];
    int len = snprintf(tmp, sizeof(tmp), fmt, f);
    Serial.write((const uint8_t*)tmp, (size_t)len);
    toAll(tmp, (size_t)len);
}

void Log::println(float f, int d) {
    char fmt[8];
    snprintf(fmt, sizeof(fmt), "%%.%df\r\n", d);
    char tmp[32];
    int len = snprintf(tmp, sizeof(tmp), fmt, f);
    Serial.print(tmp);
    toAll(tmp, (size_t)len);
}

int Log::connectedClients() {
    return numClients_;
}

void Log::toAll(const char* data, size_t len) {
    if (!telnetReady_) {
        for (size_t i = 0; i < len; i++) {
            buf_[head_] = data[i];
            head_ = (head_ + 1) % LOG_BUF_SIZE;
            if (head_ == tail_) {
                overflow_ = true;
                tail_ = (tail_ + 1) % LOG_BUF_SIZE;
            }
        }
        return;
    }

    for (int i = 0; i < numClients_; i++) {
        clients_[i].write((const uint8_t*)data, len);
    }
}

void Log::drainOne(WiFiClient& c) {
    int idx = tail_;
    while (idx != head_) {
        c.write(buf_[idx]);
        idx = (idx + 1) % LOG_BUF_SIZE;
    }
}