#pragma once
#include <Arduino.h>

// The global `Blynk` instance is defined in BlynkClient.cpp (it includes
// BlynkSimpleEsp32.h there). main.cpp must define NO_GLOBAL_BLYNK before
// including this header so the instance is not re-defined in main's TU.
class BlynkClient {
public:
    BlynkClient();

    void begin();   // Blynk.config(auth) + Blynk.connect() — WiFi must already be up
    void handle();  // Blynk.run()
    bool connected();

    // Send value to a virtual pin (V0..V255)
    void writeVp(int vpin, double value);
    void writeVp(int vpin, int value);
    void writeVp(int vpin, const char* value);

private:
    uint32_t lastReconnectMs_ = 0;
    bool began_ = false;
};