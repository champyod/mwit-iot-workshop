#pragma once
#include <Arduino.h>

class LED {
public:
    explicit LED(uint8_t pin);
    ~LED();

    uint8_t getPin() const { return pin_; }

    void on();
    void on(uint8_t brightness);  // 0–255 PWM
    void off();

    void blink(float hz, unsigned long durationMs = 0);
    void blinkCount(unsigned int count, float hz = 2);
    void stop();

    bool tick();

    static void stopAll();

private:
    uint8_t pin_;
    int8_t  pwmChannel_ = -1;
    uint8_t brightness_ = 255;

    bool    blinking_    = false;
    float   hz_          = 0;
    unsigned long durationMs_  = 0;
    unsigned int blinkCount_  = 0;
    unsigned long startMs_    = 0;
    unsigned long lastToggle_ = 0;
    bool    ledState_    = false;

    bool usePWM();
    void detachPWM();
};
