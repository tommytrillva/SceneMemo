#pragma once

#include "engine/Voice.h"
#include "engine/WavetableOsc.h"
#include "engine/ModMatrix.h"
#include "engine/ParamReader.h"
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
                       const EngineParams& params, float bpm, double ppqPosition);
    void reset();

private:
    std::array<Voice, kMaxVoices> voices;
    std::array<std::shared_ptr<const Wavetable>,
               static_cast<size_t> (Waveform::NumWaveforms)> wavetables;
    ModMatrix modMatrix;
    float sampleRate = 44100.0f;
    int blockSize = 512;

    void buildWavetables();
    void handleMidiEvent (const juce::MidiMessage& msg, const EngineParams& params);

    Voice* findFreeVoice();
    Voice* findVoiceToSteal();
    Voice* findVoicePlayingNote (int midiNote);

    void renderVoices (juce::AudioBuffer<float>& buffer, int startSample, int numSamples,
                       const EngineParams& params, float bpm, double ppqPosition);
};

} // namespace scenememo
