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
            return true;  // pressed
        }
    }
    return false;
}
