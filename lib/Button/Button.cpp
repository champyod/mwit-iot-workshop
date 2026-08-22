#include "Button.h"

Button::Button(uint8_t pin) : pin_(pin) {}

void Button::begin() {
    pinMode(pin_, INPUT_PULLUP);
}

bool Button::wasPressed() {
    const bool raw = digitalRead(pin_);
    if (raw != lastRawState_) {
        lastChangeMs_ = millis();
        lastRawState_ = raw;
    }
    if ((millis() - lastChangeMs_) > DEBOUNCE_MS && raw != lastStableState_) {
        lastStableState_ = raw;
        if (raw == LOW) {
            pressStartMs_ = millis();
            longFired_ = false;
        } else {
            if (!longFired_ && (millis() - pressStartMs_) < 3000) {
                return true;
            }
            longFired_ = false;
        }
    }
    return false;
}

bool Button::wasLongPressed(unsigned long holdMs) {
    const bool raw = digitalRead(pin_);
    if (raw != lastRawState_) {
        lastChangeMs_ = millis();
        lastRawState_ = raw;
    }
    if ((millis() - lastChangeMs_) > DEBOUNCE_MS && raw != lastStableState_) {
        lastStableState_ = raw;
        if (raw == LOW) {
            pressStartMs_ = millis();
            longFired_ = false;
        } else {
            longFired_ = false;
        }
    }
    if (!longFired_ && lastStableState_ == LOW) {
        if ((millis() - pressStartMs_) >= holdMs) {
            longFired_ = true;
            return true;
        }
    }
    return false;
}
