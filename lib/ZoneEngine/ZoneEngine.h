#pragma once
#include <Arduino.h>
#include <LED.h>
#include <UltrasonicRCW.h>

enum class RiskTier : uint8_t { SAFE, WARN, DANGER };

// Owns the RUNNING/PAUSED lifecycle and maps nearest-worker distance to
// indicator outputs. Sampling is sequential (sensor #1 → settle → sensor #2)
// to prevent ultrasonic crosstalk between modules.
class ZoneEngine {
public:
    void begin(UltrasonicRCW* us1, UltrasonicRCW* us2,
               LED* safeLed, LED* warnLed, LED* dangerLed, LED* buzzer);

    void handle();
    void setRunning(bool running);

    bool      isRunning() const { return running_; }
    float     nearestCm() const { return nearestCm_; }
    RiskTier  tier()      const { return tier_; }

    // Edge flag: true exactly once per tier transition, then resets.
    bool consumeTierChanged();

private:
    void sampleSensors();
    void applyOutputs();

    UltrasonicRCW* us1_ = nullptr;
    UltrasonicRCW* us2_ = nullptr;
    LED* safeLed_   = nullptr;
    LED* warnLed_   = nullptr;
    LED* dangerLed_ = nullptr;
    LED* buzzer_    = nullptr;

    bool     running_     = false;
    RiskTier tier_        = RiskTier::SAFE;
    bool     tierChanged_ = false;
    float    nearestCm_   = -1.0f;

    unsigned long lastSampleMs_ = 0;
};
