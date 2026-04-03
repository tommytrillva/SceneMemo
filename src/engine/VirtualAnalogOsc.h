#pragma once

#include "util/Constants.h"

namespace scenememo {

// Anti-aliased virtual analog oscillator using PolyBLEP.
class VirtualAnalogOsc
{
public:
    VirtualAnalogOsc() = default;

    void setFrequency (float freqHz, float sampleRate);
    void setShape (VAShape shape) { this->shape = shape; }
    void setPulseWidth (float pw) { pulseWidth = pw; }
    void setDrift (float amount) { driftAmount = amount; }
    void reset();

    float processSample();

private:
    VAShape shape = VAShape::Saw;
    float phase = 0.0f;
    float phaseIncrement = 0.0f;
    float pulseWidth = 0.5f;
    float sampleRate = 44100.0f;

    // Analog drift (slow random pitch modulation)
    float driftAmount = 0.0f;
    float driftPhase = 0.0f;
    float driftIncrement = 0.0f;

    float triIntegrator = 0.0f; // triangle leaky integrator state

    static float polyBlep (float t, float dt);
};

} // namespace scenememo
