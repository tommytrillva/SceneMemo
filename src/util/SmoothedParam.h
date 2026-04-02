#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>

namespace scenememo {

// Thin wrapper connecting an APVTS atomic parameter to a SmoothedValue.
// Read the atomic once per block (updateTarget), then smooth per-sample.
class SmoothedParam
{
public:
    SmoothedParam() = default;

    void init (std::atomic<float>* paramValue, float smoothingTimeSeconds, float sampleRate);
    void prepare (float sampleRate);
    void updateTarget();
    float getNextValue();
    float getCurrentValue() const;
    bool isSmoothing() const;

private:
    std::atomic<float>* source = nullptr;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothed;
    float smoothingTime = 0.02f;
};

} // namespace scenememo
