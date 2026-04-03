#pragma once

#include "util/Constants.h"
#include <array>

namespace scenememo {

class MultiModeFilter
{
public:
    MultiModeFilter() = default;

    void prepareToPlay (float sampleRate);
    void reset();

    // Process one sample through the filter.
    // keyFreq: note frequency for key tracking
    float processSample (float input, FilterType type, float cutoff, float resonance,
                         float drive, float keyTrack, float keyFreq);

private:
    float sampleRate = 44100.0f;

    // SVF state (up to 3 cascaded stages for 36dB)
    struct SVFState
    {
        float ic1eq = 0.0f;
        float ic2eq = 0.0f;
    };
    std::array<SVFState, 3> svfStages {};

    // Comb filter state
    static constexpr int kCombBufferSize = 4096;
    std::array<float, kCombBufferSize> combBuffer {};
    int combWritePos = 0;

    // Formant filter: 3 parallel bandpass filters
    struct FormantBP
    {
        SVFState state;
        float freq = 800.0f;
        float q = 5.0f;
    };
    std::array<FormantBP, 3> formantBands {};
    float formantMorph = 0.0f; // 0-1 morph between vowels

    float processSVF (SVFState& state, float input, float cutoffHz, float q, int mode);
    float processComb (float input, float cutoffHz, float feedback);
    float processFormant (float input, float morph);

    static float softClip (float x);
};

} // namespace scenememo
