#pragma once
#include <Arduino.h>

// Push button wired between pin and GND. Single state machine: poll() must be
// the only caller touching internals; take* methods consume latched events.
class Button {
public:
    explicit Button(uint8_t pin);

    void begin();

    // Advance debounce/state machine and latch short/long events. Call often.
    void poll(unsigned long holdMs = 3000);

    bool takeShortPress();
    bool takeLongPress();

    // "idle" = up, "press" = down under holdMs, "hold" = down past holdMs.
    const char* stateName() const;

    // Raw pin level, no debounce: true while the pin reads LOW (pressed).
    bool isDownRaw() const { return digitalRead(pin_) == LOW; }

private:
    uint8_t pin_;

    bool lastStableState_ = HIGH;
    unsigned long lastChangeMs_ = 0;
    unsigned long pressStartMs_ = 0;
    unsigned long holdMs_ = 3000;
    bool longFired_ = false;
    bool shortReq_ = false;
    bool longReq_ = false;

    static constexpr unsigned long DEBOUNCE_MS = 30;
};
