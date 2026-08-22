#include "LED.h"

// --- Static channel allocator ---
static int8_t nextChannel = 0;
static constexpr int MAX_CHANNELS = 16;
static LED* instances[MAX_CHANNELS] = {};

LED::LED(uint8_t pin) : pin_(pin) {}

LED::~LED() {
    stop();
    if (pwmChannel_ >= 0) {
        instances[pwmChannel_] = nullptr;
    }
}

bool LED::usePWM() {
    if (pwmChannel_ >= 0) return true;
    if (nextChannel >= MAX_CHANNELS) return false;

    pwmChannel_ = nextChannel++;
    instances[pwmChannel_] = this;

    ledcSetup(pwmChannel_, 5000, 8);
    ledcAttachPin(pin_, pwmChannel_);
    return true;
}

void LED::detachPWM() {
    if (pwmChannel_ < 0) return;
    ledcDetachPin(pin_);
    instances[pwmChannel_] = nullptr;
    pwmChannel_ = -1;
}

void LED::on() {
    stop();
    detachPWM();
    pinMode(pin_, OUTPUT);
    digitalWrite(pin_, HIGH);
    brightness_ = 255;
}

void LED::on(uint8_t brightness) {
    stop();
    brightness_ = brightness;
    if (brightness >= 255)  { on(); return; }
    if (brightness == 0)    { off(); return; }

    if (usePWM()) {
        ledcWrite(pwmChannel_, brightness);
    } else {
        pinMode(pin_, OUTPUT);
        digitalWrite(pin_, brightness > 127 ? HIGH : LOW);
    }
}

void LED::off() {
    stop();
    detachPWM();
    pinMode(pin_, OUTPUT);
    digitalWrite(pin_, LOW);
}

void LED::blinkCount(unsigned int count, float hz) {
    if (hz <= 0 || count == 0) return;
    off();
    detachPWM();
    blinking_    = true;
    hz_          = hz;
    durationMs_  = 0;
    blinkCount_  = count * 2;  // each blink = 2 toggles (ON + OFF)
    startMs_     = millis();
    lastToggle_  = startMs_;
    ledState_    = false;
    pinMode(pin_, OUTPUT);
}

void LED::blink(float hz, unsigned long durationMs) {
    if (hz <= 0) return;
    // Re-arming every control cycle would restart the phase timer and freeze
    // patterns slower than the cycle period; keep an identical run running.
    if (blinking_ && hz_ == hz && durationMs_ == durationMs) return;

    off();

    detachPWM();
    blinking_   = true;
    hz_         = hz;
    durationMs_ = durationMs;
    startMs_    = millis();
    lastToggle_ = startMs_;
    ledState_   = false;

    pinMode(pin_, OUTPUT);
}

void LED::stop() {
    blinking_ = false;
}

bool LED::tick() {
    if (!blinking_) return false;

    unsigned long now = millis();

    if (durationMs_ > 0 && now - startMs_ >= durationMs_) {
        off();
        return false;
    }

    unsigned long interval = 1000 / (hz_ * 2);
    if (interval < 1) interval = 1;

    if (now - lastToggle_ >= interval) {
        lastToggle_ = now;
        ledState_ = !ledState_;
        digitalWrite(pin_, ledState_ ? HIGH : LOW);

        if (blinkCount_ > 0) {
            blinkCount_--;
            if (blinkCount_ == 0) {
                off();
                return false;
            }
        }
    }
    return true;
}

void LED::stopAll() {
    for (int i = 0; i < MAX_CHANNELS; i++) {
        if (instances[i]) instances[i]->stop();
    }
}
