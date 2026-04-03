#include "engine/SubOsc.h"
#include <cmath>

namespace scenememo {

void SubOsc::setFrequency (float freqHz, float sampleRate)
{
    // Apply octave offset
    float subFreq = freqHz * std::pow (2.0f, static_cast<float> (octaveOffset));
    phaseIncrement = subFreq / sampleRate;
}

void SubOsc::reset()
{
    phase = 0.0f;
}

float SubOsc::processSample()
{
    float sample = 0.0f;

    switch (shape)
    {
        case SubShape::Sine:
            sample = std::sin (phase * 6.28318530718f);
            break;

        case SubShape::Triangle:
        {
            if (phase < 0.25f)       sample = phase * 4.0f;
            else if (phase < 0.75f)  sample = 2.0f - phase * 4.0f;
            else                     sample = phase * 4.0f - 4.0f;
            break;
        }

        default:
            break;
    }

    phase += phaseIncrement;
    if (phase >= 1.0f)
        phase -= 1.0f;

    return sample;
}

} // namespace scenememo
