#include "engine/NoiseOsc.h"
#include <cmath>

namespace scenememo {

void NoiseOsc::prepareToPlay (float sr)
{
    sampleRate = sr;
}

void NoiseOsc::reset()
{
    rngState = 123456789;
    pinkRows.fill (0.0f);
    pinkIndex = 0;
    pinkRunningSum = 0.0f;
    brownState = 0.0f;
    airPrev = 0.0f;
    airFilterState = 0.0f;
}

float NoiseOsc::whiteNoise()
{
    // xorshift32
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    return (static_cast<float> (rngState) / static_cast<float> (0x7FFFFFFFu)) * 2.0f - 1.0f;
}

float NoiseOsc::processSample()
{
    switch (type)
    {
        case NoiseType::White:
            return whiteNoise();

        case NoiseType::Pink:
        {
            // Voss-McCartney algorithm: update one row per sample based on
            // which bits change in the counter
            float white = whiteNoise();
            int lastIndex = pinkIndex;
            pinkIndex++;

            int diff = lastIndex ^ pinkIndex;
            for (int i = 0; i < 8; ++i)
            {
                if (diff & (1 << i))
                {
                    pinkRunningSum -= pinkRows[i];
                    pinkRows[i] = white * 0.125f; // scale per row
                    pinkRunningSum += pinkRows[i];
                    break; // only update the lowest changed bit
                }
            }
            return (pinkRunningSum + white * 0.125f) * 1.5f; // normalize roughly to [-1,1]
        }

        case NoiseType::Brown:
        {
            float white = whiteNoise();
            brownState += white * 0.02f; // small steps
            brownState *= 0.998f;        // leaky integrator (prevents DC drift)
            // Clamp to prevent runaway
            if (brownState > 1.0f) brownState = 1.0f;
            if (brownState < -1.0f) brownState = -1.0f;
            return brownState;
        }

        case NoiseType::Air:
        {
            // High-pass filtered white noise (~4kHz cutoff)
            float white = whiteNoise();
            float cutoffHz = 4000.0f;
            float rc = 1.0f / (2.0f * 3.14159265f * cutoffHz);
            float alpha = rc / (rc + 1.0f / sampleRate);
            airFilterState = alpha * (airFilterState + white - airPrev);
            airPrev = white;
            return airFilterState;
        }

        default:
            return 0.0f;
    }
}

} // namespace scenememo
