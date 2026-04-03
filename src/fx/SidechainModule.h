#pragma once

namespace scenememo {

struct SidechainParams
{
    float threshold = -20.0f;  // dB (-60 to 0)
    float attack = 1.0f;       // ms (0.1-50)
    float release = 100.0f;    // ms (10-500)
    float depth = 0.0f;        // 0-1 (how much signal ducks)
    float mix = 1.0f;          // 0-1
    bool enabled = false;
};

class SidechainModule
{
public:
    SidechainModule() = default;

    void prepareToPlay (float sampleRate);
    void reset();

    // Process with internal envelope follower (self-sidechain from low frequencies)
    void processBlock (float* left, float* right, int numSamples,
                       const SidechainParams& params);

private:
    float sampleRate = 44100.0f;
    float envelope = 0.0f;
    float lowpassState = 0.0f; // for kick detection
};

} // namespace scenememo
