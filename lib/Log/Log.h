#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#define LOG_TELNET_MAX  4
#define LOG_BUF_SIZE    2048

class Log {
public:
    Log();

    void begin(int baudRate = 115200);
    void handle();

    void print(const char* s);
    void println(const char* s);
    void printf(const char* fmt, ...) __attribute__((format(printf, 2, 3)));

    void print(int n);
    void println(int n);
    void print(float f, int d = 2);
    void println(float f, int d = 2);

    int connectedClients();

    // Stop all telnet clients (sends FIN). Call before reboot so monitors
    // see a clean EOF instead of a hung connection.
    void closeAll();

private:
    WiFiServer* server_;
    WiFiClient clients_[LOG_TELNET_MAX];
    int numClients_;
    bool telnetReady_;
    SemaphoreHandle_t mutex_;

    char buf_[LOG_BUF_SIZE];
    int head_, tail_;
    bool overflow_;

    // All public output/handle paths must hold mutex_ — the cloud-log task
    // logs concurrently with loop()'s handle().
    void lock();
    void unlock();

    void toAll(const char* data, size_t len);
    void drainOne(WiFiClient& c);
    void startTelnet();
};

extern Log Logger;