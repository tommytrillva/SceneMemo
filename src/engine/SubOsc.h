#pragma once

#include "util/Constants.h"

namespace scenememo {

class SubOsc
{
public:
    SubOsc() = default;

    void setShape (SubShape shape) { this->shape = shape; }
    void setOctaveOffset (int offset) { octaveOffset = offset; } // -1 or -2
    void setFrequency (float freqHz, float sampleRate);
    void reset();

    float processSample();

private:
    SubShape shape = SubShape::Sine;
    int octaveOffset = -1;
    float phase = 0.0f;
    float phaseIncrement = 0.0f;
};

} // namespace scenememo
