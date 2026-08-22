#pragma once
#include <Arduino.h>

// Push button wired between pin and GND. Reports one press per physical click.
class Button {
public:
    explicit Button(uint8_t pin);

    void begin();

    bool wasPressed();
    bool wasLongPressed(unsigned long holdMs = 3000);

private:
    uint8_t pin_;

    bool lastStableState_ = HIGH;
    bool lastRawState_    = HIGH;
    unsigned long lastChangeMs_ = 0;
    unsigned long pressStartMs_ = 0;
    bool longFired_ = false;

    static constexpr unsigned long DEBOUNCE_MS = 30;
};
