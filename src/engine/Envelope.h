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

private:
    State state = State::Idle;
    Parameters params;
    float sampleRate = 44100.0f;
    float currentLevel = 0.0f;

    float attackRate = 0.0f;
    float decayCoeff = 0.0f;
    float releaseCoeff = 0.0f;
    int holdSamplesRemaining = 0;

    void recalculateRates();
    float calcExpCoeff (float timeSeconds) const;
};

} // namespace scenememo
