#include "Button.h"

Button::Button(uint8_t pin) : pin_(pin) {}

void Button::begin() {
    pinMode(pin_, INPUT_PULLUP);
}

void Button::poll(unsigned long holdMs) {
    holdMs_ = holdMs;
    const bool raw = digitalRead(pin_);
    if (raw != lastStableState_) {
        if (millis() - lastChangeMs_ <= DEBOUNCE_MS && lastChangeMs_ != 0) return;
        lastChangeMs_ = millis();
        lastStableState_ = raw;
        if (raw == LOW) {
            pressStartMs_ = millis();
            longFired_ = false;
        } else if (!longFired_) {
            shortReq_ = true;
        }
    }
    if (lastStableState_ == LOW && !longFired_) {
        if (millis() - pressStartMs_ >= holdMs_) {
            longFired_ = true;
            longReq_ = true;
            shortReq_ = false;
        }
    }
}

bool Button::takeShortPress() {
    const bool v = shortReq_;
    shortReq_ = false;
    return v;
}

bool Button::takeLongPress() {
    const bool v = longReq_;
    longReq_ = false;
    return v;
}

const char* Button::stateName() const {
    if (lastStableState_ == HIGH) return "idle";
    return (millis() - pressStartMs_ >= holdMs_) ? "hold" : "press";
}

