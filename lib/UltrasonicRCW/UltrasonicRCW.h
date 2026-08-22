#pragma once
#include <Arduino.h>

// Driver for RCW-0001 micro ultrasonic module (HC-SR04-compatible protocol).
// Powered at 3V3 so ECHO is already 3.3V-safe — no divider needed.
enum class EchoStatus : uint8_t { OK, NO_ECHO, OUT_OF_RANGE };

class UltrasonicRCW {
public:
    UltrasonicRCW(uint8_t trigPin, uint8_t echoPin);

    void begin();

    // Echo listen window. RCW-0001 needs >=45 ms (its own ~42 ms latency);
    // clamped to 5..200 ms.
    void setTimeoutUs(unsigned long us);
    unsigned long echoTimeoutMs() const { return echoTimeoutUs_ / 1000UL; }

    // Blocking single shot (~timeout ms worst case). Sequential reads across
    // two modules are the caller's responsibility (crosstalk avoidance).
    // When status is non-null it receives why a read failed (-1 return):
    // NO_ECHO = pulseIn timeout, OUT_OF_RANGE = echo outside 1..450 cm.
    float readDistanceCm(EchoStatus* status = nullptr);

private:
    uint8_t trigPin_;
    uint8_t echoPin_;

    static constexpr unsigned long MIN_TIMEOUT_US = 10000UL;
    static constexpr unsigned long MAX_TIMEOUT_US = 200000UL;
    unsigned long echoTimeoutUs_ = 60000UL;
    static constexpr float MIN_VALID_CM = 1.0f;               // module blind zone
    static constexpr float MAX_VALID_CM = 450.0f;             // datasheet max @3V3
};
