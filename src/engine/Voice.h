#pragma once

#include "engine/OscillatorSlot.h"
#include "engine/MultiModeFilter.h"
#include "engine/Envelope.h"
#include "engine/LFO.h"
#include "engine/ModMatrix.h"
#include "engine/VoiceModState.h"
#include "engine/ParamReader.h"
#include "util/Constants.h"
#include <array>
#include <memory>

namespace scenememo {

class Voice
{
public:
    Voice() = default;

    void prepareToPlay (float sampleRate, int blockSize);
    void noteOn (int midiNote, float velocity, const EngineParams& params,
                 const std::array<std::shared_ptr<const Wavetable>,
                                  static_cast<size_t> (Waveform::NumWaveforms)>& wavetables);
    void noteOff();
    void reset();

    void renderBlock (float* outputL, float* outputR, int numSamples,
                      const EngineParams& params, const ModMatrix& modMatrix,
                      const std::array<std::shared_ptr<const Wavetable>,
                                       static_cast<size_t> (Waveform::NumWaveforms)>& wavetables,
                      float sampleRate, float bpm, double ppqPosition);

    bool isActive() const { return envelopes[0].isActive(); }
    int getCurrentNote() const { return currentNote; }
    AHDSREnvelope::State getEnvelopeState() const { return envelopes[0].getState(); }

    int getAge() const { return age; }
    void incrementAge() { ++age; }

    // MIDI state updates
    void setModWheel (float value) { modState.modWheel = value; }
    void setAftertouch (float value) { modState.aftertouch = value; }

private:
    std::array<OscillatorSlot, kNumOscSlots> oscSlots;
    std::array<MultiModeFilter, kNumFilters> filters;
    std::array<AHDSREnvelope, kNumEnvelopes> envelopes;
    std::array<LFO, kNumLFOs> lfos;
    VoiceModState modState;

    int currentNote = -1;
    float velocityGain = 0.0f;
    int age = 0;
    float sampleRate = 44100.0f;

    static float midiNoteToFrequency (int note, float tuneSemitones, float fineCents, float octave);
};

} // namespace scenememo
