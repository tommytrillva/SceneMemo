#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <array>

namespace scenememo {

// 4 parallel mixing lanes: Low Warmth, Mid Harmonic, High Air, Rhythm Texture
struct LaneParams
{
    float level = 0.8f;       // 0-1
    float pan = 0.0f;         // -1 to +1
    float lowCut = 20.0f;     // Hz (20-2000)
    float highCut = 20000.0f; // Hz (1000-20000)
    float stereoWidth = 1.0f; // 0-2 (0=mono, 1=normal, 2=widened)
    float fxSend = 0.0f;      // 0-1
    bool mute = false;
    bool solo = false;
};

struct MoodLanesParams
{
    std::array<LaneParams, 4> lanes;
};

class MoodLanes
{
public:
    MoodLanes();

    void prepareToPlay (float sampleRate);
    void reset();

    // Process 4 lane inputs into a stereo mix.
    // Each laneInput is a mono buffer. Output is stereo.
    void processBlock (const std::array<const float*, 4>& laneInputs,
                       float* outputL, float* outputR, int numSamples,
                       const MoodLanesParams& params);

private:
    float sampleRate = 44100.0f;

    // Simple one-pole filter states per lane
    struct FilterState
    {
        float lowCutState = 0.0f;
        float highCutState = 0.0f;
    };
    std::array<FilterState, 4> filterStates;

    static float onePoleHP (float input, float& state, float cutoffHz, float sampleRate);
    static float onePoleLP (float input, float& state, float cutoffHz, float sampleRate);
};

} // namespace scenememo
