#include "fx/SidechainModule.h"
#include <cmath>
#include <algorithm>

namespace scenememo {

void SidechainModule::prepareToPlay (float sr)
{
    sampleRate = sr;
    reset();
}

void SidechainModule::reset()
{
    envelope = 0.0f;
    lowpassState = 0.0f;
}

void SidechainModule::processBlock (float* left, float* right, int numSamples,
                                     const SidechainParams& params)
{
    if (! params.enabled || params.depth < 0.001f)
        return;

    float attackCoeff = std::exp (-1.0f / (params.attack * 0.001f * sampleRate));
    float releaseCoeff = std::exp (-1.0f / (params.release * 0.001f * sampleRate));
    float thresholdLinear = std::pow (10.0f, params.threshold / 20.0f);

    // Low-pass filter coefficient for kick detection (~150 Hz)
    float lpAlpha = (1.0f / sampleRate) / (1.0f / (6.28318530718f * 150.0f) + 1.0f / sampleRate);

    for (int i = 0; i < numSamples; ++i)
    {
        // Detect low-frequency energy (internal kick detector)
        float mono = (left[i] + right[i]) * 0.5f;
        lowpassState += lpAlpha * (mono - lowpassState);
        float sidechain = std::abs (lowpassState);

        // Envelope follower
        if (sidechain > envelope)
            envelope = attackCoeff * envelope + (1.0f - attackCoeff) * sidechain;
        else
            envelope = releaseCoeff * envelope + (1.0f - releaseCoeff) * sidechain;

        // Compute gain reduction
        float gainReduction = 1.0f;
        if (envelope > thresholdLinear)
            gainReduction = 1.0f - params.depth * std::min (1.0f, (envelope - thresholdLinear) / (thresholdLinear + 0.001f));

        gainReduction = std::clamp (gainReduction, 0.0f, 1.0f);

        // Apply with mix
        float duckedL = left[i] * gainReduction;
        float duckedR = right[i] * gainReduction;

        left[i] = left[i] * (1.0f - params.mix) + duckedL * params.mix;
        right[i] = right[i] * (1.0f - params.mix) + duckedR * params.mix;
    }
}

} // namespace scenememo
