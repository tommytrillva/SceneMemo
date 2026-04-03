#pragma once

#include "util/Constants.h"
#include <array>
#include <cstdint>

namespace scenememo {

class NoiseOsc
{
public:
    NoiseOsc() = default;

    void setType (NoiseType type) { this->type = type; }
    void prepareToPlay (float sampleRate);
    void reset();

    float processSample();

private:
    NoiseType type = NoiseType::White;
    float sampleRate = 44100.0f;

    // RNG state (xorshift32)
    uint32_t rngState = 123456789;

    // Pink noise: Voss-McCartney algorithm
    std::array<float, 8> pinkRows {};
    int pinkIndex = 0;
    float pinkRunningSum = 0.0f;

    // Brown noise: leaky integrator
    float brownState = 0.0f;

    // Air mode: HP filter state
    float airPrev = 0.0f;
    float airFilterState = 0.0f;

    float whiteNoise();
};

} // namespace scenememo
