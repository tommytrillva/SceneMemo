#pragma once

#include "engine/Voice.h"
#include "engine/WavetableOsc.h"
#include "engine/Envelope.h"
#include "util/Constants.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <memory>

namespace scenememo {

class VoiceAllocator
{
public:
    VoiceAllocator();

    void prepareToPlay (float sampleRate, int blockSize);
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi,
                       float oscLevel, float tuneSemitones, float fineCents,
                       int waveformIndex, const AHDSREnvelope::Parameters& envParams);
    void reset();

private:
    std::array<Voice, kMaxVoices> voices;
    std::array<std::shared_ptr<const Wavetable>,
               static_cast<size_t> (Waveform::NumWaveforms)> wavetables;
    float sampleRate = 44100.0f;
    int blockSize = 512;

    void buildWavetables();
    void handleMidiEvent (const juce::MidiMessage& msg, int waveformIndex);

    Voice* findFreeVoice();
    Voice* findVoiceToSteal();
    Voice* findVoicePlayingNote (int midiNote);

    // Render all active voices into the buffer range [startSample, startSample + numSamples)
    void renderVoices (juce::AudioBuffer<float>& buffer, int startSample, int numSamples,
                       float oscLevel, float tuneSemitones, float fineCents);
};

} // namespace scenememo
