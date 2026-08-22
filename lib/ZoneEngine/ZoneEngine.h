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

    // Live config — thresholds and per-sensor calibration (live-only, no NVS).
    void  setThresholds(float dangerCm, float warnCm);
    float dangerThresh() const { return dangerThresh_; }
    float warnThresh()   const { return warnThresh_; }

    void  setCalibration(uint8_t idx, float offsetCm, float scale);
    float calibrationOffset(uint8_t idx) const;
    float calibrationScale(uint8_t idx) const;

    float   sensorCm(uint8_t idx) const;
    float   sensorRawCm(uint8_t idx) const;
    uint8_t sensorCount() const { return 2; }

    void resetDefaults();

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

    float dangerThresh_ = 50.0f;
    float warnThresh_   = 100.0f;
    float offsets_[2]   = {0.0f, 0.0f};
    float scales_[2]    = {1.0f, 1.0f};
    float raw_[2]       = {-1.0f, -1.0f};
    float corrected_[2] = {-1.0f, -1.0f};
};
