#pragma once

#include "engine/WavetableOsc.h"
#include "engine/Envelope.h"
#include <memory>

namespace scenememo {

class Voice
{
public:
    Voice() = default;

    void prepareToPlay (float sampleRate, int blockSize);
    void noteOn (int midiNote, float velocity, std::shared_ptr<const Wavetable> wt);
    void noteOff();
    void reset();

    // Render this voice into the output buffers (additive).
    void renderBlock (float* outputL, float* outputR, int numSamples,
                      float oscLevel, float tuneSemitones, float fineCents,
                      float sampleRate);

    bool isActive() const { return envelope.isActive(); }
    int getCurrentNote() const { return currentNote; }
    AHDSREnvelope::State getEnvelopeState() const { return envelope.getState(); }

    void setEnvelopeParameters (const AHDSREnvelope::Parameters& params);

    // Age tracking for voice stealing (incremented each processBlock)
    int getAge() const { return age; }
    void incrementAge() { ++age; }

private:
    WavetableOsc oscillator;
    AHDSREnvelope envelope;

    int currentNote = -1;
    float velocityGain = 0.0f;
    int age = 0;
    float sampleRate = 44100.0f;

    static float midiNoteToFrequency (int note, float tuneSemitones, float fineCents);
};

} // namespace scenememo
