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

    void setEnabled(uint8_t idx, bool enabled);
    bool isEnabled(uint8_t idx) const;

    // Timing — live-only. Interval = ms between detection cycles (floor 10).
    // Delay = pre-ping wait before that sensor fires within the cycle.
    void          setSampleIntervalMs(unsigned long ms);
    unsigned long sampleIntervalMs() const { return sampleIntervalMs_; }
    void          setSensorDelayMs(uint8_t idx, unsigned long ms);
    unsigned long sensorDelayMs(uint8_t idx) const;

    float   sensorCm(uint8_t idx) const;
    float   sensorRawCm(uint8_t idx) const;
    uint8_t sensorCount() const { return 2; }

    EchoStatus  sensorStatus(uint8_t idx) const;
    uint16_t    sensorFailStreak(uint8_t idx) const;

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

    unsigned long sampleIntervalMs_   = 100;
    unsigned long sensorDelayMs_[2]   = {10, 10};

    float dangerThresh_ = 50.0f;
    float warnThresh_   = 100.0f;
    bool  enabled_[2]   = {true, true};
    float offsets_[2]   = {0.0f, 0.0f};
    float scales_[2]    = {1.0f, 1.0f};
    float raw_[2]       = {-1.0f, -1.0f};
    float corrected_[2] = {-1.0f, -1.0f};

    EchoStatus status_[2]     = {EchoStatus::NO_ECHO, EchoStatus::NO_ECHO};
    uint16_t   failStreak_[2] = {0, 0};
};
