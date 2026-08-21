#pragma once
#include <Arduino.h>

// Driver for RCW-0001 micro ultrasonic module (HC-SR04-compatible protocol).
// Powered at 3V3 so ECHO is already 3.3V-safe — no divider needed.
class UltrasonicRCW {
public:
    UltrasonicRCW(uint8_t trigPin, uint8_t echoPin);

    void begin();

    // Blocking single shot (~30 ms worst case). Sequential reads across two
    // modules are the caller's responsibility (crosstalk avoidance).
    float readDistanceCm();

private:
    uint8_t trigPin_;
    uint8_t echoPin_;

    static constexpr unsigned long ECHO_TIMEOUT_US = 30000UL; // ~5 m round trip
    static constexpr float MIN_VALID_CM = 1.0f;               // module blind zone
    static constexpr float MAX_VALID_CM = 450.0f;             // datasheet max @3V3
};
