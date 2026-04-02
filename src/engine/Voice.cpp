#include "engine/Voice.h"
#include <cmath>

namespace scenememo {

void Voice::prepareToPlay (float sr, int /*blockSize*/)
{
    sampleRate = sr;
    envelope.setSampleRate (sr);
}

void Voice::noteOn (int midiNote, float velocity, std::shared_ptr<const Wavetable> wt)
{
    currentNote = midiNote;
    velocityGain = velocity;
    age = 0;

    oscillator.setWavetable (std::move (wt));
    oscillator.reset();
    envelope.noteOn();
}

void Voice::noteOff()
{
    envelope.noteOff();
}

void Voice::reset()
{
    currentNote = -1;
    velocityGain = 0.0f;
    age = 0;
    envelope.reset();
    oscillator.reset();
}

void Voice::setEnvelopeParameters (const AHDSREnvelope::Parameters& params)
{
    envelope.setParameters (params);
}

void Voice::renderBlock (float* outputL, float* outputR, int numSamples,
                         float oscLevel, float tuneSemitones, float fineCents,
                         float sr)
{
    if (! isActive())
        return;

    float freq = midiNoteToFrequency (currentNote, tuneSemitones, fineCents);
    oscillator.setFrequency (freq, sr);

    float gain = oscLevel * velocityGain;

    for (int i = 0; i < numSamples; ++i)
    {
        float oscSample = oscillator.processSample();
        float envValue = envelope.processSample();
        float out = oscSample * envValue * gain;

        outputL[i] += out;
        outputR[i] += out;

        // If envelope just went idle, stop processing remaining samples
        if (! envelope.isActive())
            break;
    }
}

float Voice::midiNoteToFrequency (int note, float tuneSemitones, float fineCents)
{
    float semitones = static_cast<float> (note) - 69.0f + tuneSemitones + fineCents / 100.0f;
    return 440.0f * std::pow (2.0f, semitones / 12.0f);
}

} // namespace scenememo
