#include "DHT22_Sensor.h"

DHT22_Sensor::DHT22_Sensor(uint8_t pin)
    : pin_(pin), dht_(pin, DHT22) {}

bool DHT22_Sensor::begin() {
    dht_.begin();
    Serial.printf("[DHT22] Sensor on pin %d\n", pin_);
    return true;
}

DHT22_Sensor::Data DHT22_Sensor::read() {
    Data data;

    float h = dht_.readHumidity();
    float t = dht_.readTemperature();
    float f = dht_.readTemperature(true);

    if (isnan(h) || isnan(t) || isnan(f)) {
        Serial.printf("[DHT22] Read failed on pin %d\n", pin_);
        return data;
    }

    data.temperature = t;
    data.humidity    = h;
    data.heatIndex   = dht_.computeHeatIndex(t, h, false);
    data.valid       = true;

    Serial.printf("[DHT22] %.1f C | %.1f %% | HI %.1f C\n",
                  data.temperature, data.humidity, data.heatIndex);
    return data;
}
