#pragma once
#include <Arduino.h>
#include <DHT.h>

class DHT22_Sensor {
public:
    explicit DHT22_Sensor(uint8_t pin);

    bool begin();

    struct Data {
        float temperature = 0;
        float humidity    = 0;
        float heatIndex   = 0;
        bool  valid       = false;
    };

    Data read();
    uint8_t getPin() const { return pin_; }

private:
    uint8_t pin_;
    DHT dht_;
};
