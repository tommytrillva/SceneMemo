#include "engine/OscillatorSlot.h"
#include <cmath>
#include <algorithm>

namespace scenememo {

OscillatorSlot::OscillatorSlot()
{
    // Initialize all unison slots as WavetableOsc by default
    for (auto& osc : unisonOscs)
        osc = WavetableOsc();
}

void OscillatorSlot::prepareToPlay (float sr)
{
    sampleRate = sr;
    // Prepare noise oscillators if present
    for (auto& osc : unisonOscs)
    {
        if (auto* noise = std::get_if<NoiseOsc> (&osc))
            noise->prepareToPlay (sr);
    }
}

void OscillatorSlot::setType (OscType type)
{
    if (type == currentType)
        return;
    currentType = type;
    ensureType (type);
}

void OscillatorSlot::ensureType (OscType type)
{
    for (auto& osc : unisonOscs)
    {
        switch (type)
        {
            case OscType::Wavetable:     osc = WavetableOsc(); break;
            case OscType::VirtualAnalog: osc = VirtualAnalogOsc(); break;
            case OscType::Noise:
            {
                NoiseOsc n;
                n.prepareToPlay (sampleRate);
                osc = n;
                break;
            }
            case OscType::Sub:           osc = SubOsc(); break;
            default:                     osc = WavetableOsc(); break;
        }
    }
}

void OscillatorSlot::reset()
{
    for (auto& osc : unisonOscs)
    {
        std::visit ([] (auto& o) {
            o.reset();
        }, osc);
    }
}

void OscillatorSlot::configureOsc (OscVariant& osc, OscType type, float freq, float sr,
                                   int waveform, float /*wtPos*/,
                                   const std::array<std::shared_ptr<const Wavetable>,
                                                    static_cast<size_t> (Waveform::NumWaveforms)>& wavetables)
{
    switch (type)
    {
        case OscType::Wavetable:
            if (auto* wt = std::get_if<WavetableOsc> (&osc))
            {
                int wtIdx = std::clamp (waveform, 0, static_cast<int> (Waveform::NumWaveforms) - 1);
                wt->setWavetable (wavetables[static_cast<size_t> (wtIdx)]);
                wt->setFrequency (freq, sr);
            }
            break;

        case OscType::VirtualAnalog:
            if (auto* va = std::get_if<VirtualAnalogOsc> (&osc))
            {
                va->setShape (static_cast<VAShape> (std::clamp (waveform, 0,
                    static_cast<int> (VAShape::NumShapes) - 1)));
                va->setFrequency (freq, sr);
            }
            break;

        case OscType::Noise:
            if (auto* n = std::get_if<NoiseOsc> (&osc))
            {
                n->setType (static_cast<NoiseType> (std::clamp (waveform, 0,
                    static_cast<int> (NoiseType::NumTypes) - 1)));
            }
            break;

        case OscType::Sub:
            if (auto* sub = std::get_if<SubOsc> (&osc))
            {
                sub->setShape (static_cast<SubShape> (std::clamp (waveform, 0,
                    static_cast<int> (SubShape::NumShapes) - 1)));
                sub->setFrequency (freq, sr);
            }
            break;

        default:
            break;
    }
}

float OscillatorSlot::processOscSample (OscVariant& osc)
{
    return std::visit ([] (auto& o) -> float {
        return o.processSample();
    }, osc);
}

void OscillatorSlot::renderBlock (float* outputL, float* outputR, int numSamples,
                                  float freq, float level, float pan, float sr,
                                  int unisonCount, float unisonSpread,
                                  int waveform, float wtPos,
                                  const std::array<std::shared_ptr<const Wavetable>,
                                                   static_cast<size_t> (Waveform::NumWaveforms)>& wavetables)
{
    if (level < 0.0001f)
        return;

    unisonCount = std::clamp (unisonCount, 1, kMaxUnisonVoices);

    // Pan → gain for left/right (constant power panning)
    float panAngle = (pan + 1.0f) * 0.5f; // 0..1
    float gainL = std::cos (panAngle * 1.5707963f) * level;
    float gainR = std::sin (panAngle * 1.5707963f) * level;

    // Scale per voice to maintain consistent volume
    float unisonGain = 1.0f / std::sqrt (static_cast<float> (unisonCount));

    // Calculate detuning for each unison voice
    for (int u = 0; u < unisonCount; ++u)
    {
        // Spread detuning: centered around base freq
        float detuneAmount = 0.0f;
        if (unisonCount > 1)
        {
            float normalized = (static_cast<float> (u) / static_cast<float> (unisonCount - 1)) * 2.0f - 1.0f;
            detuneAmount = normalized * unisonSpread * 30.0f; // max ±30 cents
        }

        float detunedFreq = freq * std::pow (2.0f, detuneAmount / 1200.0f);

        // Stereo spread for unison voices
        float uPan = 0.0f;
        if (unisonCount > 1)
        {
            float normalized = (static_cast<float> (u) / static_cast<float> (unisonCount - 1)) * 2.0f - 1.0f;
            uPan = normalized * unisonSpread;
        }

        float uPanAngle = (uPan + 1.0f) * 0.5f;
        float uGainL = std::cos (uPanAngle * 1.5707963f) * unisonGain;
        float uGainR = std::sin (uPanAngle * 1.5707963f) * unisonGain;

        configureOsc (unisonOscs[u], currentType, detunedFreq, sr, waveform, wtPos, wavetables);

        for (int i = 0; i < numSamples; ++i)
        {
            float sample = processOscSample (unisonOscs[u]);
            outputL[i] += sample * gainL * uGainL;
            outputR[i] += sample * gainR * uGainR;
        }
    }
}

} // namespace scenememo
