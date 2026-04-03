#include "engine/Voice.h"
#include <cmath>
#include <cstring>
#include <algorithm>

namespace scenememo {

void Voice::prepareToPlay (float sr, int /*blockSize*/)
{
    sampleRate = sr;
    for (auto& env : envelopes)
        env.setSampleRate (sr);
    for (auto& lfo : lfos)
        lfo.prepareToPlay (sr);
    for (auto& slot : oscSlots)
        slot.prepareToPlay (sr);
    for (auto& filt : filters)
        filt.prepareToPlay (sr);
}

void Voice::noteOn (int midiNote, float velocity, const EngineParams& params,
                    const std::array<std::shared_ptr<const Wavetable>,
                                     static_cast<size_t> (Waveform::NumWaveforms)>& /*wavetables*/)
{
    currentNote = midiNote;
    velocityGain = velocity;
    age = 0;

    // Set up modulation state
    modState.velocity = velocity;
    modState.keyPosition = static_cast<float> (midiNote) / 127.0f;
    // Generate random value for this note
    modState.randomValue = static_cast<float> (std::rand()) / static_cast<float> (RAND_MAX);

    // Trigger all envelopes
    for (int i = 0; i < kNumEnvelopes; ++i)
    {
        AHDSREnvelope::Parameters envParams;
        envParams.attack       = params.env[i].attack;
        envParams.hold         = params.env[i].hold;
        envParams.decay        = params.env[i].decay;
        envParams.sustain      = params.env[i].sustain;
        envParams.release      = params.env[i].release;
        envParams.attackCurve  = params.env[i].attackCurve;
        envParams.decayCurve   = params.env[i].decayCurve;
        envParams.releaseCurve = params.env[i].releaseCurve;
        envelopes[i].setParameters (envParams);
        envelopes[i].noteOn();
    }

    // Set up oscillator slot types
    for (int i = 0; i < kNumOscSlots; ++i)
        oscSlots[i].setType (static_cast<OscType> (params.osc[i].type));

    // Reset oscillator phases
    for (auto& slot : oscSlots)
        slot.reset();

    // Trigger LFOs
    for (int i = 0; i < kNumLFOs; ++i)
        lfos[i].noteOn (params.lfo[i].phase, static_cast<LFORetrigger> (params.lfo[i].retrigger));
}

void Voice::noteOff()
{
    for (auto& env : envelopes)
        env.noteOff();
}

void Voice::reset()
{
    currentNote = -1;
    velocityGain = 0.0f;
    age = 0;
    for (auto& env : envelopes)
        env.reset();
    for (auto& lfo : lfos)
        lfo.reset();
    for (auto& slot : oscSlots)
        slot.reset();
    for (auto& filt : filters)
        filt.reset();
    modState.clear();
}

void Voice::renderBlock (float* outputL, float* outputR, int numSamples,
                         const EngineParams& params, const ModMatrix& modMatrix,
                         const std::array<std::shared_ptr<const Wavetable>,
                                          static_cast<size_t> (Waveform::NumWaveforms)>& wavetables,
                         float sr, float bpm, double ppqPosition)
{
    if (! isActive())
        return;

    // Update envelope parameters
    for (int i = 0; i < kNumEnvelopes; ++i)
    {
        AHDSREnvelope::Parameters envParams;
        envParams.attack       = params.env[i].attack;
        envParams.hold         = params.env[i].hold;
        envParams.decay        = params.env[i].decay;
        envParams.sustain      = params.env[i].sustain;
        envParams.release      = params.env[i].release;
        envParams.attackCurve  = params.env[i].attackCurve;
        envParams.decayCurve   = params.env[i].decayCurve;
        envParams.releaseCurve = params.env[i].releaseCurve;
        envelopes[i].setParameters (envParams);
    }

    // Process in sub-blocks of kModBlockSize for modulation
    int samplesRemaining = numSamples;
    int offset = 0;

    while (samplesRemaining > 0 && isActive())
    {
        int blockLen = std::min (samplesRemaining, kModBlockSize);

        // --- Evaluate modulation sources (once per sub-block) ---
        float envValues[kNumEnvelopes];
        for (int i = 0; i < kNumEnvelopes; ++i)
            envValues[i] = envelopes[i].getCurrentLevel();

        float lfoValues[kNumLFOs];
        for (int i = 0; i < kNumLFOs; ++i)
            lfoValues[i] = lfos[i].getCurrentValue();

        ModMatrix::fillSources (modState, envValues, kNumEnvelopes,
                                lfoValues, kNumLFOs,
                                params.macro.data(), kNumMacros);
        modMatrix.evaluate (modState, params);

        // --- Render oscillators into a temp buffer ---
        float tempL[kModBlockSize] = {};
        float tempR[kModBlockSize] = {};

        float keyFreq = midiNoteToFrequency (currentNote, 0, 0, 0);

        for (int o = 0; o < kNumOscSlots; ++o)
        {
            const auto& oscP = params.osc[o];
            float modLevel = ModMatrix::getDestOffset (modState, static_cast<ModDest> (
                static_cast<int> (ModDest::Osc1Level) + o * 4));
            float modFine  = ModMatrix::getDestOffset (modState, static_cast<ModDest> (
                static_cast<int> (ModDest::Osc1Fine) + o * 4));
            float modPan   = ModMatrix::getDestOffset (modState, static_cast<ModDest> (
                static_cast<int> (ModDest::Osc1Pan) + o * 4));

            float level = std::clamp (oscP.level + modLevel, 0.0f, 1.0f);
            if (level < 0.0001f)
                continue;

            float freq = midiNoteToFrequency (currentNote, oscP.tune, oscP.fine + modFine * 100.0f, oscP.octave);
            float pan = std::clamp (oscP.pan + modPan, -1.0f, 1.0f);

            oscSlots[o].setType (static_cast<OscType> (oscP.type));
            oscSlots[o].renderBlock (tempL, tempR, blockLen,
                                     freq, level, pan, sr,
                                     oscP.unison, oscP.unisonSpread,
                                     oscP.waveform, oscP.wtPos, wavetables);
        }

        // --- Apply filters ---
        int filtRouting = params.filtRouting;

        for (int i = 0; i < blockLen; ++i)
        {
            float mixedMono = (tempL[i] + tempR[i]) * 0.5f;
            float filtL = tempL[i];
            float filtR = tempR[i];

            auto applyFilter = [&] (int filtIdx, float input) -> float
            {
                const auto& fp = params.filt[filtIdx];
                float modCutoff = ModMatrix::getDestOffset (modState, static_cast<ModDest> (
                    static_cast<int> (ModDest::Filt1Cutoff) + filtIdx * 3));
                float modReso   = ModMatrix::getDestOffset (modState, static_cast<ModDest> (
                    static_cast<int> (ModDest::Filt1Reso) + filtIdx * 3));

                // Apply envelope amount (env2 for filter typically)
                float envMod = (kNumEnvelopes > 1) ? envelopes[1].getCurrentLevel() * fp.envAmt : 0.0f;
                float cutoff = std::clamp (fp.cutoff + modCutoff * 10000.0f + envMod * 10000.0f, 20.0f, 20000.0f);
                float reso = std::clamp (fp.reso + modReso, 0.0f, 1.0f);

                return filters[filtIdx].processSample (
                    input, static_cast<FilterType> (fp.type),
                    cutoff, reso, fp.drive, fp.keyTrack, keyFreq);
            };

            if (filtRouting == static_cast<int> (FilterRouting::Series))
            {
                filtL = applyFilter (0, filtL);
                filtL = applyFilter (1, filtL);
                filtR = applyFilter (0, filtR);
                filtR = applyFilter (1, filtR);
            }
            else if (filtRouting == static_cast<int> (FilterRouting::Parallel))
            {
                float f1L = applyFilter (0, filtL);
                float f2L = applyFilter (1, filtL);
                float f1R = applyFilter (0, filtR);
                float f2R = applyFilter (1, filtR);
                filtL = (f1L + f2L) * 0.5f;
                filtR = (f1R + f2R) * 0.5f;
            }
            else // Split: osc1+2 → filt1, osc3+4 → filt2 (approximation: just apply both)
            {
                filtL = applyFilter (0, filtL);
                filtR = applyFilter (1, filtR);
            }

            tempL[i] = filtL;
            tempR[i] = filtR;
        }

        // --- Apply amplitude envelope and write to output ---
        for (int i = 0; i < blockLen; ++i)
        {
            float ampEnv = envelopes[0].processSample();
            float gain = ampEnv * velocityGain;

            outputL[offset + i] += tempL[i] * gain;
            outputR[offset + i] += tempR[i] * gain;

            // Tick remaining envelopes (2-4, for modulation sources)
            for (int e = 1; e < kNumEnvelopes; ++e)
                envelopes[e].processSample();

            // Tick LFOs
            for (int l = 0; l < kNumLFOs; ++l)
            {
                const auto& lp = params.lfo[l];
                lfos[l].processSample (
                    static_cast<LFOShape> (lp.shape), lp.rate, lp.sync,
                    lp.syncRate, lp.fadeIn, lp.humanize, bpm, ppqPosition);
            }

            if (! envelopes[0].isActive())
                break;
        }

        offset += blockLen;
        samplesRemaining -= blockLen;
    }
}

float Voice::midiNoteToFrequency (int note, float tuneSemitones, float fineCents, float octave)
{
    float semitones = static_cast<float> (note) - 69.0f + tuneSemitones + fineCents / 100.0f + octave * 12.0f;
    return 440.0f * std::pow (2.0f, semitones / 12.0f);
}

} // namespace scenememo
