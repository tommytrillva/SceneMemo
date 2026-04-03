#pragma once

#include <array>

namespace scenememo {

struct DelayParams
{
    float time = 0.25f;       // seconds (tempo-synced or free)
    float feedback = 0.3f;    // 0-1
    float mix = 0.0f;         // 0-1
    float filterLP = 8000.0f; // feedback LP filter Hz
    float filterHP = 200.0f;  // feedback HP filter Hz
    float diffusion = 0.0f;   // 0-1 (smears repeats)
    int stereoMode = 0;       // 0=mono, 1=ping-pong, 2=dual
    bool tempoSync = false;
};

class DelayEffect
{
public:
    DelayEffect();

    void prepareToPlay (float sampleRate);
    void reset();

    void processBlock (float* left, float* right, int numSamples,
                       const DelayParams& params, float bpm);

private:
    float sampleRate = 44100.0f;

    static constexpr int kMaxDelaySamples = 192000 * 4; // ~4 seconds at 192kHz
    std::array<float, kMaxDelaySamples> bufferL {};
    std::array<float, kMaxDelaySamples> bufferR {};
    int writePos = 0;

    // Feedback filter states
    float fbLPStateL = 0.0f, fbLPStateR = 0.0f;
    float fbHPStateL = 0.0f, fbHPStateR = 0.0f;

    float readFromBuffer (const std::array<float, kMaxDelaySamples>& buf, float delaySamples) const;
};

} // namespace scenememo
