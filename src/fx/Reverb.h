#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace scenememo {

struct ReverbParams
{
    float size = 0.5f;
    float decay = 2.0f;
    float damping = 0.5f;
    float predelay = 20.0f;
    float modDepth = 0.0f;
    float highCut = 10000.0f;
    float lowCut = 100.0f;
    float mix = 0.3f;
    bool infinite = false;
};

class ReverbEffect
{
public:
    ReverbEffect();

    void prepareToPlay (float sampleRate, int blockSize);
    void reset();

    void processBlock (float* left, float* right, int numSamples, const ReverbParams& params);

private:
    juce::Reverb reverb;
    juce::Reverb::Parameters juceReverbParams;
    float sampleRate = 44100.0f;
};

} // namespace scenememo
