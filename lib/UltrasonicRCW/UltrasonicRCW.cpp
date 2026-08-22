#include "UltrasonicRCW.h"

UltrasonicRCW::UltrasonicRCW(uint8_t trigPin, uint8_t echoPin)
    : trigPin_(trigPin), echoPin_(echoPin) {}

void UltrasonicRCW::begin() {
    pinMode(trigPin_, OUTPUT);
    digitalWrite(trigPin_, LOW);
    pinMode(echoPin_, INPUT);
}

void UltrasonicRCW::setTimeoutUs(unsigned long us) {
    if (us < MIN_TIMEOUT_US) us = MIN_TIMEOUT_US;
    if (us > MAX_TIMEOUT_US) us = MAX_TIMEOUT_US;
    echoTimeoutUs_ = us;
}

float UltrasonicRCW::readDistanceCm(EchoStatus* status) {
    digitalWrite(trigPin_, LOW);
    delayMicroseconds(4);
    digitalWrite(trigPin_, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin_, LOW);

    const unsigned long durationUs = pulseIn(echoPin_, HIGH, echoTimeoutUs_);
    if (durationUs == 0) {
        if (status) *status = EchoStatus::NO_ECHO;
        return -1.0f;
    }

    const float cm = static_cast<float>(durationUs) / 58.0f;
    if (cm < MIN_VALID_CM || cm > MAX_VALID_CM) {
        if (status) *status = EchoStatus::OUT_OF_RANGE;
        return -1.0f;
    }
    if (status) *status = EchoStatus::OK;
    return cm;
}
