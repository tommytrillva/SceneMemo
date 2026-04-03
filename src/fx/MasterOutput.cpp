#include "fx/MasterOutput.h"
#include <cmath>
#include <algorithm>

namespace scenememo {

void MasterOutput::prepareToPlay (float sr)
{
    sampleRate = sr;
    reset();
}

void MasterOutput::reset()
{
    eqLowStateL = eqLowStateR = 0.0f;
    eqHighStateL = eqHighStateR = 0.0f;
}

float MasterOutput::softLimit (float x, float ceiling)
{
    float ceilLin = std::pow (10.0f, ceiling / 20.0f);
    if (std::abs (x) <= ceilLin)
        return x;

    // Soft knee limiter using tanh
    float normalized = x / ceilLin;
    return std::tanh (normalized) * ceilLin;
}

void MasterOutput::processBlock (float* left, float* right, int numSamples,
                                  const MasterOutputParams& params)
{
    // Pre-compute EQ gains
    float lowGain = std::pow (10.0f, params.eqLowGain / 20.0f);
    float midGain = std::pow (10.0f, params.eqMidGain / 20.0f);
    float highGain = std::pow (10.0f, params.eqHighGain / 20.0f);

    // EQ filter coefficients (low shelf ~200Hz, high shelf ~4kHz)
    float lowAlpha = (1.0f / sampleRate) / (1.0f / (6.28318530718f * 200.0f) + 1.0f / sampleRate);
    float highAlpha = (1.0f / sampleRate) / (1.0f / (6.28318530718f * 4000.0f) + 1.0f / sampleRate);

    for (int i = 0; i < numSamples; ++i)
    {
        float L = left[i];
        float R = right[i];

        // --- 3-band EQ (simplified shelving) ---
        // Low shelf
        eqLowStateL += lowAlpha * (L - eqLowStateL);
        eqLowStateR += lowAlpha * (R - eqLowStateR);
        float lowL = eqLowStateL;
        float lowR = eqLowStateR;

        // High shelf
        eqHighStateL += highAlpha * (L - eqHighStateL);
        eqHighStateR += highAlpha * (R - eqHighStateR);
        float highL = L - eqHighStateL;
        float highR = R - eqHighStateR;

        // Mid = original - low - high
        float midL = L - lowL - highL;
        float midR = R - lowR - highR;

        L = lowL * lowGain + midL * midGain + highL * highGain;
        R = lowR * lowGain + midR * midGain + highR * highGain;

        // --- Stereo Width (mid/side processing) ---
        if (std::abs (params.stereoWidth - 1.0f) > 0.01f)
        {
            float mid = (L + R) * 0.5f;
            float side = (L - R) * 0.5f;

            side *= params.stereoWidth;

            L = mid + side;
            R = mid - side;
        }

        // --- Soft Limiter ---
        if (params.limiterEnabled)
        {
            L = softLimit (L, params.limiterCeiling);
            R = softLimit (R, params.limiterCeiling);
        }

        // --- Output Level ---
        L *= params.outputLevel;
        R *= params.outputLevel;

        left[i] = L;
        right[i] = R;
    }
}

} // namespace scenememo
