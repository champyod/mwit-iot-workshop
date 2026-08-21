#include "UltrasonicRCW.h"

UltrasonicRCW::UltrasonicRCW(uint8_t trigPin, uint8_t echoPin)
    : trigPin_(trigPin), echoPin_(echoPin) {}

void UltrasonicRCW::begin() {
    pinMode(trigPin_, OUTPUT);
    digitalWrite(trigPin_, LOW);
    pinMode(echoPin_, INPUT);
}

float UltrasonicRCW::readDistanceCm() {
    digitalWrite(trigPin_, LOW);
    delayMicroseconds(4);
    digitalWrite(trigPin_, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin_, LOW);

    const unsigned long durationUs = pulseIn(echoPin_, HIGH, ECHO_TIMEOUT_US);
    if (durationUs == 0) {
        return -1.0f;  // no echo within timeout window
    }

    const float cm = static_cast<float>(durationUs) / 58.0f;
    if (cm < MIN_VALID_CM || cm > MAX_VALID_CM) {
        return -1.0f;  // outside datasheet range → invalid reading
    }
    return cm;
}
