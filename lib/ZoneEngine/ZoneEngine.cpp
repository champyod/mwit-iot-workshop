#include "ZoneEngine.h"

namespace {
constexpr unsigned long SAMPLE_INTERVAL_MS = 100;
constexpr unsigned long SENSOR_SETTLE_MS   = 10;
constexpr float WARN_THRESHOLD_CM   = 100.0f;
constexpr float DANGER_THRESHOLD_CM = 50.0f;
constexpr float LED_WARN_BLINK_HZ   = 2.0f;
constexpr float BUZZER_DANGER_HZ    = 6.0f;

RiskTier classifyTier(float cm) {
    if (cm < 0.0f) {
        return RiskTier::SAFE;  // no valid reading → fail-safe to SAFE
    }
    if (cm <= DANGER_THRESHOLD_CM) return RiskTier::DANGER;
    if (cm <= WARN_THRESHOLD_CM)   return RiskTier::WARN;
    return RiskTier::SAFE;
}
}  // namespace

void ZoneEngine::begin(UltrasonicRCW* us1, UltrasonicRCW* us2,
                       LED* safeLed, LED* warnLed, LED* dangerLed, LED* buzzer) {
    us1_ = us1;
    us2_ = us2;
    safeLed_ = safeLed;
    warnLed_ = warnLed;
    dangerLed_ = dangerLed;
    buzzer_ = buzzer;
    setRunning(false);
}

void ZoneEngine::setRunning(bool running) {
    running_ = running;
    if (!running) {
        safeLed_->stop();   safeLed_->off();
        warnLed_->stop();   warnLed_->off();
        dangerLed_->stop(); dangerLed_->off();
        buzzer_->stop();    buzzer_->off();
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
    if (millis() - lastSampleMs_ < SAMPLE_INTERVAL_MS) return;
    lastSampleMs_ = millis();

    sampleSensors();
    applyOutputs();
}

void ZoneEngine::sampleSensors() {
    const float d1 = us1_->readDistanceCm();
    delay(SENSOR_SETTLE_MS);  // let first module's echo ring decay before second ping
    const float d2 = us2_->readDistanceCm();

    float nearest = -1.0f;
    if (d1 >= 0.0f && d2 >= 0.0f) {
        nearest = (d1 < d2) ? d1 : d2;
    } else if (d1 >= 0.0f) {
        nearest = d1;
    } else if (d2 >= 0.0f) {
        nearest = d2;
    }
    nearestCm_ = nearest;

    const RiskTier next = classifyTier(nearest);
    tierChanged_ = (next != tier_);
    tier_ = next;
}

void ZoneEngine::applyOutputs() {
    switch (tier_) {
    case RiskTier::DANGER:
        safeLed_->stop();   safeLed_->off();
        warnLed_->stop();   warnLed_->off();
        dangerLed_->on();
        buzzer_->blink(BUZZER_DANGER_HZ);
        break;
    case RiskTier::WARN:
        safeLed_->stop();   safeLed_->off();
        dangerLed_->stop(); dangerLed_->off();
        buzzer_->stop();    buzzer_->off();
        warnLed_->blink(LED_WARN_BLINK_HZ);
        break;
    case RiskTier::SAFE:
        warnLed_->stop();   warnLed_->off();
        dangerLed_->stop(); dangerLed_->off();
        buzzer_->stop();    buzzer_->off();
        safeLed_->on();
        break;
    }
}

bool ZoneEngine::consumeTierChanged() {
    const bool changed = tierChanged_;
    tierChanged_ = false;
    return changed;
}
