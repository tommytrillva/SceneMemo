#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "util/Constants.h"
#include "engine/ParameterLayout.h"
#include <array>
#include <atomic>

namespace scenememo {

// Flat parameter snapshot — read once per block, no atomics in render loop.
struct EngineParams
{
    // Global
    bool bypass = false;
    float masterVol = 0.8f;
    float dryWet = 1.0f;

    // Oscillators
    struct OscParams
    {
        int type = 0;       // OscType
        int waveform = 0;   // shape index
        float level = 0.0f;
        float pan = 0.0f;
        float tune = 0.0f;  // semitones
        float fine = 0.0f;  // cents
        float octave = 0.0f;
        float wtPos = 0.0f;
        int unison = 1;
        float unisonSpread = 0.5f;
    };
    std::array<OscParams, kNumOscSlots> osc;

    // Filters
    struct FilterParams
    {
        int type = 1;        // FilterType (default LP24)
        float cutoff = 20000.0f;
        float reso = 0.0f;
        float drive = 0.0f;
        float keyTrack = 0.0f;
        float envAmt = 0.0f;
    };
    std::array<FilterParams, kNumFilters> filt;
    int filtRouting = 0; // FilterRouting

    // Envelopes
    struct EnvParams
    {
        float attack = 0.01f;
        float hold = 0.0f;
        float decay = 0.3f;
        float sustain = 0.7f;
        float release = 0.3f;
        float attackCurve = 0.0f;
        float decayCurve = 0.0f;
        float releaseCurve = 0.0f;
    };
    std::array<EnvParams, kNumEnvelopes> env;

    // LFOs
    struct LfoParams
    {
        int shape = 0;
        float rate = 1.0f;
        bool sync = false;
        int syncRate = 9; // 1/4 note
        float phase = 0.0f;
        float fadeIn = 0.0f;
        int retrigger = 0;
        float humanize = 0.0f;
    };
    std::array<LfoParams, kNumLFOs> lfo;

    // Mod matrix
    struct ModSlotParams
    {
        int source = 0;
        int dest = 0;
        float depth = 0.0f;
    };
    std::array<ModSlotParams, kMaxModSlots> mod;

    // Macros
    std::array<float, kNumMacros> macro = {};
};

// Caches all APVTS atomic pointers at construction. Reads all params in one call.
class ParamReader
{
public:
    ParamReader() = default;
    void cachePointers (juce::AudioProcessorValueTreeState& apvts);
    void readAll (EngineParams& out) const;

private:
    // Global
    std::atomic<float>* bypass = nullptr;
    std::atomic<float>* masterVol = nullptr;
    std::atomic<float>* dryWet = nullptr;

    // Oscillators
    struct OscPtrs {
        std::atomic<float>* type = nullptr;
        std::atomic<float>* waveform = nullptr;
        std::atomic<float>* level = nullptr;
        std::atomic<float>* pan = nullptr;
        std::atomic<float>* tune = nullptr;
        std::atomic<float>* fine = nullptr;
        std::atomic<float>* octave = nullptr;
        std::atomic<float>* wtPos = nullptr;
        std::atomic<float>* unison = nullptr;
        std::atomic<float>* unisonSpread = nullptr;
    };
    std::array<OscPtrs, kNumOscSlots> oscPtrs;

    // Filters
    struct FilterPtrs {
        std::atomic<float>* type = nullptr;
        std::atomic<float>* cutoff = nullptr;
        std::atomic<float>* reso = nullptr;
        std::atomic<float>* drive = nullptr;
        std::atomic<float>* keyTrack = nullptr;
        std::atomic<float>* envAmt = nullptr;
    };
    std::array<FilterPtrs, kNumFilters> filtPtrs;
    std::atomic<float>* filtRouting = nullptr;

    // Envelopes
    struct EnvPtrs {
        std::atomic<float>* attack = nullptr;
        std::atomic<float>* hold = nullptr;
        std::atomic<float>* decay = nullptr;
        std::atomic<float>* sustain = nullptr;
        std::atomic<float>* release = nullptr;
        std::atomic<float>* attackCurve = nullptr;
        std::atomic<float>* decayCurve = nullptr;
        std::atomic<float>* releaseCurve = nullptr;
    };
    std::array<EnvPtrs, kNumEnvelopes> envPtrs;

    // LFOs
    struct LfoPtrs {
        std::atomic<float>* shape = nullptr;
        std::atomic<float>* rate = nullptr;
        std::atomic<float>* sync = nullptr;
        std::atomic<float>* syncRate = nullptr;
        std::atomic<float>* phase = nullptr;
        std::atomic<float>* fadeIn = nullptr;
        std::atomic<float>* retrigger = nullptr;
        std::atomic<float>* humanize = nullptr;
    };
    std::array<LfoPtrs, kNumLFOs> lfoPtrs;

    // Mod matrix
    struct ModPtrs {
        std::atomic<float>* source = nullptr;
        std::atomic<float>* dest = nullptr;
        std::atomic<float>* depth = nullptr;
    };
    std::array<ModPtrs, kMaxModSlots> modPtrs;

    // Macros
    std::array<std::atomic<float>*, kNumMacros> macroPtrs = {};
};

} // namespace scenememo
