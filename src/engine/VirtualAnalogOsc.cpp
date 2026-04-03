#include "engine/VirtualAnalogOsc.h"
#include <cmath>

namespace scenememo {

void VirtualAnalogOsc::setFrequency (float freqHz, float sr)
{
    sampleRate = sr;
    phaseIncrement = freqHz / sr;

    // Drift LFO: slow random oscillation (~0.5 Hz)
    driftIncrement = 0.5f / sr;
}

void VirtualAnalogOsc::reset()
{
    phase = 0.0f;
    driftPhase = 0.0f;
}

float VirtualAnalogOsc::polyBlep (float t, float dt)
{
    // PolyBLEP correction for discontinuities
    if (t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.0f;
    }
    else if (t > 1.0f - dt)
    {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    return 0.0f;
}

float VirtualAnalogOsc::processSample()
{
    // Apply drift
    float effectiveInc = phaseIncrement;
    if (driftAmount > 0.0f)
    {
        float driftValue = std::sin (driftPhase * 6.28318530718f);
        float driftCents = driftValue * driftAmount * 10.0f; // max ±10 cents at full drift
        effectiveInc *= std::pow (2.0f, driftCents / 1200.0f);
        driftPhase += driftIncrement;
        if (driftPhase >= 1.0f)
            driftPhase -= 1.0f;
    }

    float sample = 0.0f;
    float dt = effectiveInc;

    switch (shape)
    {
        case VAShape::Sine:
            sample = std::sin (phase * 6.28318530718f);
            break;

        case VAShape::Saw:
        {
            sample = 2.0f * phase - 1.0f;
            sample -= polyBlep (phase, dt);
            break;
        }

        case VAShape::Square:
        {
            sample = phase < 0.5f ? 1.0f : -1.0f;
            sample += polyBlep (phase, dt);
            sample -= polyBlep (std::fmod (phase + 0.5f, 1.0f), dt);
            break;
        }

        case VAShape::Triangle:
        {
            // Integrated square wave → triangle
            sample = phase < 0.5f ? 1.0f : -1.0f;
            sample += polyBlep (phase, dt);
            sample -= polyBlep (std::fmod (phase + 0.5f, 1.0f), dt);
            // Leaky integrator to get triangle from square
            triIntegrator = dt * sample + (1.0f - dt) * triIntegrator;
            sample = triIntegrator * 4.0f;
            break;
        }

        case VAShape::Pulse:
        {
            sample = phase < pulseWidth ? 1.0f : -1.0f;
            sample += polyBlep (phase, dt);
            sample -= polyBlep (std::fmod (phase + (1.0f - pulseWidth), 1.0f), dt);
            break;
        }

        default:
            break;
    }

    // Advance phase
    phase += effectiveInc;
    if (phase >= 1.0f)
        phase -= 1.0f;

    return sample;
}

} // namespace scenememo
