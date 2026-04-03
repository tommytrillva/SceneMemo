#pragma once

#include <cstdint>

namespace scenememo {

struct GritParams
{
    // Tape saturation
    float tapeDrive = 0.0f;     // 0-1
    float tapeBias = 0.5f;      // 0-1 (even vs odd harmonics)
    int tapeSpeed = 1;          // 0=7.5ips, 1=15ips, 2=30ips

    // Vinyl character
    float vinylNoise = 0.0f;    // 0-1
    float vinylCrackle = 0.0f;  // 0-1
    float wowFlutter = 0.0f;    // 0-1 depth
    float wowRate = 0.5f;       // Hz

    // Bit crusher
    float bitDepth = 24.0f;     // 1-24
    float sampleRateReduce = 44100.0f; // 100-44100

    // Decade macro: 0 = 70s, 1 = modern
    float decade = 1.0f;
};

class GritSection
{
public:
    GritSection();

    void prepareToPlay (float sampleRate);
    void reset();

    void processBlock (float* left, float* right, int numSamples, const GritParams& params);

private:
    float sampleRate = 44100.0f;

    // Tape saturation state
    float tapeFilterState = 0.0f;

    // Vinyl noise RNG
    uint32_t rngState = 777;

    // Wow & flutter LFO
    float wowPhase = 0.0f;

    // Bit crusher state
    float crushHoldL = 0.0f;
    float crushHoldR = 0.0f;
    float crushCounter = 0.0f;

    float nextRng();
    static float tapeWaveshaper (float input, float drive, float bias);
    static float softClip (float x);
};

} // namespace scenememo
