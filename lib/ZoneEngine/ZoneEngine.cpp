#include "ZoneEngine.h"

namespace {
constexpr float LED_WARN_BLINK_HZ = 2.0f;
constexpr float BUZZER_DANGER_HZ = 6.0f;
constexpr unsigned long MAX_SAMPLE_INTERVAL_MS = 60000UL;
constexpr unsigned long MIN_TIMING_MS = 10UL;
constexpr unsigned long MAX_SENSOR_DELAY_MS = 5000UL;

RiskTier classifyTier(float cm, float dangerThresh, float warnThresh) {
    if (cm < 0.0f) return RiskTier::SAFE;
    if (cm <= dangerThresh) return RiskTier::DANGER;
    if (cm <= warnThresh) return RiskTier::WARN;
    return RiskTier::SAFE;
}

float clamp(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}
}

void ZoneEngine::begin(UltrasonicRCW* us1, UltrasonicRCW* us2,
                       LED* safeLed, LED* warnLed, LED* dangerLed, LED* buzzer) {
    us1_ = us1;
    us2_ = us2;
    safeLed_ = safeLed;
    warnLed_ = warnLed;
    dangerLed_ = dangerLed;
    buzzer_ = buzzer;
    resetDefaults();
    setRunning(false);
}

void ZoneEngine::setRunning(bool running) {
    running_ = running;
    if (!running) {
        safeLed_->stop(); safeLed_->off();
        warnLed_->stop(); warnLed_->off();
        dangerLed_->stop(); dangerLed_->off();
        buzzer_->stop(); buzzer_->off();
        nearestCm_ = -1.0f;
        tier_ = RiskTier::SAFE;
        tierChanged_ = false;
    }
}

void ZoneEngine::handle() {
    safeLed_->tick();
    warnLed_->tick();
    dangerLed_->tick();
    buzzer_->tick();
    if (!running_) return;
    if (millis() - lastSampleMs_ < sampleIntervalMs_) return;
    lastSampleMs_ = millis();
    sampleSensors();
    applyOutputs();
}

void ZoneEngine::sampleSensors() {
    EchoStatus s1 = EchoStatus::NO_ECHO;
    EchoStatus s2 = EchoStatus::NO_ECHO;
    if (enabled_[0]) {
        delay(sensorDelayMs_[0]);
        raw_[0] = us1_->readDistanceCm(&s1);
        status_[0] = s1;
    } else {
        raw_[0] = -1.0f;
    }
    if (enabled_[1]) {
        delay(sensorDelayMs_[1]);
        raw_[1] = us2_->readDistanceCm(&s2);
        status_[1] = s2;
    } else {
        raw_[1] = -1.0f;
    }
    for (uint8_t i = 0; i < 2; ++i) {
        const float raw = raw_[i];
        if (raw < 0.0f) {
            if (failStreak_[i] < 65535) ++failStreak_[i];
            corrected_[i] = -1.0f;
        } else {
            failStreak_[i] = 0;
            float v = (raw + offsets_[i]) * scales_[i];
            corrected_[i] = (v < 0.0f) ? -1.0f : v;
        }
    }
    float nearest = -1.0f;
    if (corrected_[0] >= 0.0f && corrected_[1] >= 0.0f) {
        nearest = (corrected_[0] < corrected_[1]) ? corrected_[0] : corrected_[1];
    } else if (corrected_[0] >= 0.0f) {
        nearest = corrected_[0];
    } else if (corrected_[1] >= 0.0f) {
        nearest = corrected_[1];
    }
    nearestCm_ = nearest;
    const RiskTier next = classifyTier(nearest, dangerThresh_, warnThresh_);
    tierChanged_ = (next != tier_);
    tier_ = next;
}

void ZoneEngine::applyOutputs() {
    switch (tier_) {
    case RiskTier::DANGER:
        safeLed_->stop(); safeLed_->off();
        warnLed_->stop(); warnLed_->off();
        dangerLed_->on();
        buzzer_->blink(BUZZER_DANGER_HZ);
        break;
    case RiskTier::WARN:
        safeLed_->stop(); safeLed_->off();
        dangerLed_->stop(); dangerLed_->off();
        buzzer_->stop(); buzzer_->off();
        warnLed_->blink(LED_WARN_BLINK_HZ);
        break;
    case RiskTier::SAFE:
        warnLed_->stop(); warnLed_->off();
        dangerLed_->stop(); dangerLed_->off();
        buzzer_->stop(); buzzer_->off();
        safeLed_->on();
        break;
    }
}

bool ZoneEngine::consumeTierChanged() {
    const bool changed = tierChanged_;
    tierChanged_ = false;
    return changed;
}

void ZoneEngine::setThresholds(float dangerCm, float warnCm) {
    dangerCm = clamp(dangerCm, 5.0f, 400.0f);
    warnCm = clamp(warnCm, 5.0f, 450.0f);
    if (warnCm <= dangerCm) warnCm = dangerCm + 5.0f;
    if (warnCm > 450.0f) warnCm = 450.0f;
    dangerThresh_ = dangerCm;
    warnThresh_ = warnCm;
}

void ZoneEngine::setCalibration(uint8_t idx, float offsetCm, float scale) {
    if (idx >= 2) return;
    offsets_[idx] = clamp(offsetCm, -50.0f, 50.0f);
    scales_[idx] = clamp(scale, 0.5f, 2.0f);
}

float ZoneEngine::calibrationOffset(uint8_t idx) const {
    if (idx >= 2) return 0.0f;
    return offsets_[idx];
}

float ZoneEngine::calibrationScale(uint8_t idx) const {
    if (idx >= 2) return 1.0f;
    return scales_[idx];
}

float ZoneEngine::sensorCm(uint8_t idx) const {
    if (idx >= 2) return -1.0f;
    return corrected_[idx];
}

float ZoneEngine::sensorRawCm(uint8_t idx) const {
    if (idx >= 2) return -1.0f;
    return raw_[idx];
}

EchoStatus ZoneEngine::sensorStatus(uint8_t idx) const {
    if (idx >= 2) return EchoStatus::NO_ECHO;
    return status_[idx];
}

uint16_t ZoneEngine::sensorFailStreak(uint8_t idx) const {
    if (idx >= 2) return 0;
    return failStreak_[idx];
}

void ZoneEngine::setEnabled(uint8_t idx, bool enabled) {
    if (idx >= 2) return;
    enabled_[idx] = enabled;
    failStreak_[idx] = 0;
}

bool ZoneEngine::isEnabled(uint8_t idx) const {
    if (idx >= 2) return false;
    return enabled_[idx];
}

void ZoneEngine::setSampleIntervalMs(unsigned long ms) {
    if (ms < MIN_TIMING_MS) ms = MIN_TIMING_MS;
    if (ms > MAX_SAMPLE_INTERVAL_MS) ms = MAX_SAMPLE_INTERVAL_MS;
    sampleIntervalMs_ = ms;
}

void ZoneEngine::setSensorDelayMs(uint8_t idx, unsigned long ms) {
    if (idx >= 2) return;
    if (ms < MIN_TIMING_MS) ms = MIN_TIMING_MS;
    if (ms > MAX_SENSOR_DELAY_MS) ms = MAX_SENSOR_DELAY_MS;
    sensorDelayMs_[idx] = ms;
}

unsigned long ZoneEngine::sensorDelayMs(uint8_t idx) const {
    if (idx >= 2) return 0;
    return sensorDelayMs_[idx];
}

void ZoneEngine::resetDefaults() {
    dangerThresh_ = 50.0f;
    warnThresh_ = 100.0f;
    offsets_[0] = 0.0f; offsets_[1] = 0.0f;
    scales_[0] = 1.0f; scales_[1] = 1.0f;
    failStreak_[0] = 0; failStreak_[1] = 0;
    enabled_[0] = true; enabled_[1] = true;
    sampleIntervalMs_ = 100;
    sensorDelayMs_[0] = 10; sensorDelayMs_[1] = 10;
}
