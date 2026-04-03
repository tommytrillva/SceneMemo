#pragma once

#include "util/Constants.h"
#include <array>
#include <cstdint>

namespace scenememo {

class LFO
{
public:
    LFO() = default;

    void prepareToPlay (float sampleRate);
    void noteOn (float phaseOffset, LFORetrigger retrigger);
    void reset();

    // Process one sample. Returns value in [-1, +1].
    // bpm and ppqPosition come from the host's AudioPlayHead.
    float processSample (LFOShape shape, float rate, bool tempoSync,
                         int syncDivIndex, float fadeInTime, float humanize,
                         float bpm, double ppqPosition);

    float getCurrentValue() const { return currentValue; }

private:
    float sampleRate = 44100.0f;
    float phase = 0.0f;
    float currentValue = 0.0f;
    float fadeInLevel = 1.0f;
    float fadeInRate = 0.0f;
    bool firstSample = true;

    // S&H / random smooth state
    float prevRandom = 0.0f;
    float nextRandom = 0.0f;
    uint32_t rngState = 12345;

    float nextRng();
    float computeShape (LFOShape shape, float ph) const;
};

} // namespace scenememo
