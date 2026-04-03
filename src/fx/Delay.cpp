#include "fx/Delay.h"
#include <cmath>
#include <algorithm>

namespace scenememo {

DelayEffect::DelayEffect() = default;

void DelayEffect::prepareToPlay (float sr)
{
    sampleRate = sr;
    reset();
}

void DelayEffect::reset()
{
    bufferL.fill (0.0f);
    bufferR.fill (0.0f);
    writePos = 0;
    fbLPStateL = fbLPStateR = 0.0f;
    fbHPStateL = fbHPStateR = 0.0f;
}

float DelayEffect::readFromBuffer (const std::array<float, kMaxDelaySamples>& buf, float delaySamples) const
{
    float readPosFloat = static_cast<float> (writePos) - delaySamples;
    if (readPosFloat < 0.0f)
        readPosFloat += static_cast<float> (kMaxDelaySamples);

    int idx = static_cast<int> (readPosFloat);
    float frac = readPosFloat - static_cast<float> (idx);
    int next = (idx + 1) % kMaxDelaySamples;
    idx = idx % kMaxDelaySamples;

    return buf[idx] * (1.0f - frac) + buf[next] * frac;
}

void DelayEffect::processBlock (float* left, float* right, int numSamples,
                                 const DelayParams& params, float bpm)
{
    if (params.mix < 0.001f)
        return;

    float delayTimeSec = params.time;
    float delaySamples = delayTimeSec * sampleRate;
    delaySamples = std::clamp (delaySamples, 1.0f, static_cast<float> (kMaxDelaySamples - 1));

    float feedback = std::clamp (params.feedback, 0.0f, 0.95f); // safety limiter

    // Filter coefficients for feedback path
    float lpAlpha = (1.0f / sampleRate) / (1.0f / (6.28318530718f * params.filterLP) + 1.0f / sampleRate);
    float hpRC = 1.0f / (6.28318530718f * params.filterHP);
    float hpAlpha = hpRC / (hpRC + 1.0f / sampleRate);

    for (int i = 0; i < numSamples; ++i)
    {
        float dryL = left[i];
        float dryR = right[i];

        float delayedL = readFromBuffer (bufferL, delaySamples);
        float delayedR = readFromBuffer (bufferR, delaySamples);

        // Apply feedback filtering
        fbLPStateL += lpAlpha * (delayedL - fbLPStateL);
        fbLPStateR += lpAlpha * (delayedR - fbLPStateR);
        float filteredL = fbLPStateL;
        float filteredR = fbLPStateR;

        // HP on feedback
        fbHPStateL = hpAlpha * (fbHPStateL + filteredL - fbHPStateL);
        fbHPStateR = hpAlpha * (fbHPStateR + filteredR - fbHPStateR);
        filteredL = filteredL - fbHPStateL;
        filteredR = filteredR - fbHPStateR;

        // Stereo modes
        float fbL, fbR;
        if (params.stereoMode == 1) // ping-pong
        {
            fbL = filteredR * feedback;
            fbR = filteredL * feedback;
        }
        else // mono or dual
        {
            fbL = filteredL * feedback;
            fbR = filteredR * feedback;
        }

        // Write to delay buffer
        bufferL[writePos] = dryL + fbL;
        bufferR[writePos] = dryR + fbR;
        writePos = (writePos + 1) % kMaxDelaySamples;

        // Mix
        left[i] = dryL * (1.0f - params.mix) + delayedL * params.mix;
        right[i] = dryR * (1.0f - params.mix) + delayedR * params.mix;
    }
}

} // namespace scenememo
