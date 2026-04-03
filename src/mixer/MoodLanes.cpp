#include "mixer/MoodLanes.h"
#include <cmath>
#include <algorithm>

namespace scenememo {

MoodLanes::MoodLanes() = default;

void MoodLanes::prepareToPlay (float sr)
{
    sampleRate = sr;
    reset();
}

void MoodLanes::reset()
{
    for (auto& fs : filterStates)
    {
        fs.lowCutState = 0.0f;
        fs.highCutState = 0.0f;
    }
}

float MoodLanes::onePoleHP (float input, float& state, float cutoffHz, float sr)
{
    float rc = 1.0f / (2.0f * 3.14159265f * cutoffHz);
    float alpha = rc / (rc + 1.0f / sr);
    state = alpha * (state + input - state);
    return input - state;
}

float MoodLanes::onePoleLP (float input, float& state, float cutoffHz, float sr)
{
    float rc = 1.0f / (2.0f * 3.14159265f * cutoffHz);
    float alpha = (1.0f / sr) / (rc + 1.0f / sr);
    state += alpha * (input - state);
    return state;
}

void MoodLanes::processBlock (const std::array<const float*, 4>& laneInputs,
                               float* outputL, float* outputR, int numSamples,
                               const MoodLanesParams& params)
{
    // Check for any solo'd lane
    bool anySolo = false;
    for (const auto& lp : params.lanes)
        if (lp.solo) anySolo = true;

    for (int i = 0; i < numSamples; ++i)
    {
        float mixL = 0.0f;
        float mixR = 0.0f;

        for (int lane = 0; lane < 4; ++lane)
        {
            const auto& lp = params.lanes[lane];

            // Mute/solo logic
            if (lp.mute) continue;
            if (anySolo && !lp.solo) continue;

            float sample = (laneInputs[lane] != nullptr) ? laneInputs[lane][i] : 0.0f;

            // Apply filters
            sample = onePoleHP (sample, filterStates[lane].lowCutState, lp.lowCut, sampleRate);
            sample = onePoleLP (sample, filterStates[lane].highCutState, lp.highCut, sampleRate);

            // Apply level
            sample *= lp.level;

            // Pan (constant power)
            float panAngle = (lp.pan + 1.0f) * 0.5f;
            float gainL = std::cos (panAngle * 1.5707963f);
            float gainR = std::sin (panAngle * 1.5707963f);

            // Stereo width (mid/side processing simplified)
            float mid = sample;
            float side = 0.0f; // mono input, so side = 0
            float widthMid = mid * (2.0f - lp.stereoWidth);
            float widthSide = side * lp.stereoWidth;
            float wL = widthMid + widthSide;
            float wR = widthMid - widthSide;

            mixL += wL * gainL;
            mixR += wR * gainR;
        }

        outputL[i] += mixL;
        outputR[i] += mixR;
    }
}

} // namespace scenememo
