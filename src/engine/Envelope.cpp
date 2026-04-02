#include "engine/Envelope.h"
#include <cmath>
#include <algorithm>

namespace scenememo {

void AHDSREnvelope::setParameters (const Parameters& p)
{
    params = p;
    recalculateRates();
}

void AHDSREnvelope::setSampleRate (float sr)
{
    sampleRate = sr;
    recalculateRates();
}

void AHDSREnvelope::noteOn()
{
    // Start attack from current level (smooth retrigger)
    state = State::Attack;
    recalculateRates();
}

void AHDSREnvelope::noteOff()
{
    if (state != State::Idle)
    {
        state = State::Release;
        recalculateRates();
    }
}

void AHDSREnvelope::reset()
{
    state = State::Idle;
    currentLevel = 0.0f;
    holdSamplesRemaining = 0;
}

float AHDSREnvelope::processSample()
{
    switch (state)
    {
        case State::Idle:
            return 0.0f;

        case State::Attack:
            currentLevel += attackRate;
            if (currentLevel >= 1.0f)
            {
                currentLevel = 1.0f;
                state = State::Hold;
                holdSamplesRemaining = static_cast<int> (params.hold * sampleRate);
            }
            break;

        case State::Hold:
            if (--holdSamplesRemaining <= 0)
                state = State::Decay;
            break;

        case State::Decay:
        {
            // Exponential decay toward sustain level
            float target = params.sustain;
            currentLevel = target + (currentLevel - target) * decayCoeff;

            // Transition when close enough to sustain
            if (std::abs (currentLevel - target) < 0.0001f)
            {
                currentLevel = target;
                state = State::Sustain;
            }
            break;
        }

        case State::Sustain:
            currentLevel = params.sustain;
            break;

        case State::Release:
            currentLevel *= releaseCoeff;
            if (currentLevel < 0.0001f)
            {
                currentLevel = 0.0f;
                state = State::Idle;
            }
            break;
    }

    return currentLevel;
}

void AHDSREnvelope::recalculateRates()
{
    // Linear attack rate: reach 1.0 in attack time
    float attackSamples = std::max (1.0f, params.attack * sampleRate);
    attackRate = 1.0f / attackSamples;

    // Exponential coefficients for decay and release
    decayCoeff = calcExpCoeff (params.decay);
    releaseCoeff = calcExpCoeff (params.release);
}

float AHDSREnvelope::calcExpCoeff (float timeSeconds) const
{
    // Coefficient per sample for exponential decay.
    // After 'timeSeconds', signal reaches ~0.001 of original (-60dB).
    if (timeSeconds <= 0.0f)
        return 0.0f;

    float numSamples = timeSeconds * sampleRate;
    if (numSamples < 1.0f)
        return 0.0f;

    // exp(-6.9 / N) gives ~0.001 after N samples (e^-6.9 ≈ 0.001)
    return std::exp (-6.9078f / numSamples);
}

} // namespace scenememo
