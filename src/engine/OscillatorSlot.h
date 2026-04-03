#pragma once

#include "engine/WavetableOsc.h"
#include "engine/VirtualAnalogOsc.h"
#include "engine/NoiseOsc.h"
#include "engine/SubOsc.h"
#include "util/Constants.h"
#include <variant>
#include <array>
#include <memory>

namespace scenememo {

using OscVariant = std::variant<WavetableOsc, VirtualAnalogOsc, NoiseOsc, SubOsc>;

class OscillatorSlot
{
public:
    OscillatorSlot();

    void prepareToPlay (float sampleRate);
    void setType (OscType type);
    void reset();

    // Render this slot into the output buffers (additive).
    // freq is the base frequency (already note + coarse tune + fine + octave applied).
    void renderBlock (float* outputL, float* outputR, int numSamples,
                      float freq, float level, float pan, float sampleRate,
                      int unisonCount, float unisonSpread,
                      int waveform, float wtPos,
                      const std::array<std::shared_ptr<const Wavetable>,
                                       static_cast<size_t> (Waveform::NumWaveforms)>& wavetables);

private:
    OscType currentType = OscType::Wavetable;
    float sampleRate = 44100.0f;

    // Unison: up to kMaxUnisonVoices sub-oscillators
    std::array<OscVariant, kMaxUnisonVoices> unisonOscs;

    void ensureType (OscType type);
    void configureOsc (OscVariant& osc, OscType type, float freq, float sr,
                       int waveform, float wtPos,
                       const std::array<std::shared_ptr<const Wavetable>,
                                        static_cast<size_t> (Waveform::NumWaveforms)>& wavetables);
    static float processOscSample (OscVariant& osc);
};

} // namespace scenememo
