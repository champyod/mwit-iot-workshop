#pragma once
#include <Arduino.h>

// Push button wired between pin and GND. Reports one press per physical click.
class Button {
public:
    explicit Button(uint8_t pin);

    void begin();

    // Call every loop pass. True exactly once per debounced falling edge.
    bool wasPressed();

private:
    uint8_t pin_;

    bool lastStableState_ = HIGH;
    bool lastRawState_    = HIGH;
    unsigned long lastChangeMs_ = 0;

    static constexpr unsigned long DEBOUNCE_MS = 30;
};
