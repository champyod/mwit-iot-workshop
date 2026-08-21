#pragma once
#include <Arduino.h>

// ---- RCW-0001 ultrasonic #1 ----
constexpr uint8_t PIN_US1_TRIG = 5;
constexpr uint8_t PIN_US1_ECHO = 18;

// ---- RCW-0001 ultrasonic #2 ----
constexpr uint8_t PIN_US2_TRIG = 23;
constexpr uint8_t PIN_US2_ECHO = 19;

// ---- Risk indicator LEDs ----
constexpr uint8_t PIN_LED_SAFE   = 26;  // blue LED = SAFE tier
constexpr uint8_t PIN_LED_WARN   = 27;  // yellow LED = WARN tier
constexpr uint8_t PIN_LED_DANGER = 25;  // red LED module = DANGER tier

// ---- Active buzzer ----
constexpr uint8_t PIN_BUZZER = 14;

// ---- Engine play/pause button (to GND, INPUT_PULLUP) ----
constexpr uint8_t PIN_BUTTON = 32;
