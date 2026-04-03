#pragma once

#include "util/Constants.h"

namespace scenememo {

class AHDSREnvelope
{
public:
    struct Parameters
    {
        float attack  = kDefaultAttack;
        float hold    = kDefaultHold;
        float decay   = kDefaultDecay;
        float sustain = kDefaultSustain;
        float release = kDefaultRelease;
        // Curve shapes: -1 = concave, 0 = linear/default, +1 = convex
        float attackCurve  = 0.0f;
        float decayCurve   = 0.0f;
        float releaseCurve = 0.0f;
    };

    enum class State { Idle, Attack, Hold, Decay, Sustain, Release };

    AHDSREnvelope() = default;

    void setParameters (const Parameters& params);
    void setSampleRate (float sr);
    void noteOn();
    void noteOff();
    void reset();

    float processSample();
    bool isActive() const { return state != State::Idle; }
    State getState() const { return state; }
    float getCurrentLevel() const { return currentLevel; }

private:
    State state = State::Idle;
    Parameters params;
    float sampleRate = 44100.0f;
    float currentLevel = 0.0f;

    // Attack state
    float attackRate = 0.0f;
    float attackProgress = 0.0f;   // 0..1 linear progress through attack
    float attackStartLevel = 0.0f; // for smooth retrigger

    // Decay/release exponential coefficients
    float decayCoeff = 0.0f;
    float releaseCoeff = 0.0f;

    int holdSamplesRemaining = 0;

    void recalculateRates();
    float calcExpCoeff (float timeSeconds) const;
    static float shapeCurve (float linearValue, float curveAmount);
};

} // namespace scenememo
