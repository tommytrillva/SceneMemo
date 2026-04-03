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
    attackStartLevel = currentLevel;
    attackProgress = 0.0f;
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
    attackProgress = 0.0f;
    attackStartLevel = 0.0f;
    holdSamplesRemaining = 0;
}

float AHDSREnvelope::processSample()
{
    switch (state)
    {
        case State::Idle:
            return 0.0f;

        case State::Attack:
        {
            attackProgress += attackRate;
            if (attackProgress >= 1.0f)
            {
                attackProgress = 1.0f;
                currentLevel = 1.0f;
                state = State::Hold;
                holdSamplesRemaining = static_cast<int> (params.hold * sampleRate);
            }
            else
            {
                float shaped = shapeCurve (attackProgress, params.attackCurve);
                currentLevel = attackStartLevel + (1.0f - attackStartLevel) * shaped;
            }
            break;
        }

        case State::Hold:
            if (--holdSamplesRemaining <= 0)
                state = State::Decay;
            break;

        case State::Decay:
        {
            float target = params.sustain;
            currentLevel = target + (currentLevel - target) * decayCoeff;

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
    float attackSamples = std::max (1.0f, params.attack * sampleRate);
    attackRate = 1.0f / attackSamples;

    // Apply curve to exponential coefficients:
    // Positive curve = faster initial decay (more convex), negative = slower (more concave)
    float decayTime = params.decay * std::pow (2.0f, -params.decayCurve * 0.5f);
    float releaseTime = params.release * std::pow (2.0f, -params.releaseCurve * 0.5f);

    decayCoeff = calcExpCoeff (decayTime);
    releaseCoeff = calcExpCoeff (releaseTime);
}

float AHDSREnvelope::calcExpCoeff (float timeSeconds) const
{
    if (timeSeconds <= 0.0f)
        return 0.0f;

    float numSamples = timeSeconds * sampleRate;
    if (numSamples < 1.0f)
        return 0.0f;

    return std::exp (-6.9078f / numSamples);
}

float AHDSREnvelope::shapeCurve (float linearValue, float curveAmount)
{
    // curveAmount: -1 = concave (fast start, slow finish)
    //               0 = linear
    //              +1 = convex (slow start, fast finish)
    if (std::abs (curveAmount) < 0.01f)
        return linearValue;

    // Map curve to exponent: 2^(curve*2) gives range [0.25, 4]
    float exponent = std::pow (2.0f, curveAmount * 2.0f);
    return std::pow (linearValue, exponent);
}

} // namespace scenememo
