#pragma once

#include <cstdint>

namespace scenememo {

// ============================================================================
// Audio Engine Constants
// ============================================================================

constexpr int kMaxVoices = 8;
constexpr int kDefaultWavetableSize = 2048;
constexpr int kMaxMipLevels = 10;
constexpr float kMinFrequency = 20.0f;
constexpr float kMaxFrequency = 20000.0f;
constexpr float kParamSmoothingSeconds = 0.02f;

// Phase 2: engine dimensions
constexpr int kNumOscSlots = 4;
constexpr int kNumFilters = 2;
constexpr int kNumEnvelopes = 4;
constexpr int kNumLFOs = 4;
constexpr int kMaxModSlots = 16;
constexpr int kNumMacros = 4;
constexpr int kModBlockSize = 32;
constexpr int kMaxUnisonVoices = 8;

// Modulation system
constexpr int kMaxModSources = 24;
constexpr int kMaxModDestinations = 64;

// ============================================================================
// Envelope Defaults & Limits
// ============================================================================

constexpr float kDefaultAttack  = 0.01f;
constexpr float kDefaultHold    = 0.0f;
constexpr float kDefaultDecay   = 0.3f;
constexpr float kDefaultSustain = 0.7f;
constexpr float kDefaultRelease = 0.3f;

constexpr float kMinEnvTime = 0.001f;
constexpr float kMaxAttack  = 5.0f;
constexpr float kMaxHold    = 2.0f;
constexpr float kMaxDecay   = 5.0f;
constexpr float kMaxRelease = 10.0f;

// ============================================================================
// Enums
// ============================================================================

// Wavetable waveform types (used by WavetableOsc)
enum class Waveform : int {
    Sine = 0, Saw, Square, Triangle, NumWaveforms
};

// Oscillator slot type
enum class OscType : int {
    Wavetable = 0, VirtualAnalog, Noise, Sub, NumTypes
};

// Virtual Analog shapes
enum class VAShape : int {
    Sine = 0, Saw, Square, Triangle, Pulse, NumShapes
};

// Noise types
enum class NoiseType : int {
    White = 0, Pink, Brown, Air, NumTypes
};

// Sub oscillator shapes
enum class SubShape : int {
    Sine = 0, Triangle, NumShapes
};

// Filter types
enum class FilterType : int {
    LP12 = 0, LP24, LP36, HP12, HP24, BP, Notch, Comb, Formant, NumTypes
};

// Filter routing
enum class FilterRouting : int {
    Series = 0, Parallel, Split, NumRoutings
};

// LFO shapes
enum class LFOShape : int {
    Sine = 0, Triangle, SawUp, SawDown, Square,
    SampleHold, RandomSmooth, Custom32, NumShapes
};

// LFO retrigger mode
enum class LFORetrigger : int {
    Free = 0, NoteOn, FirstNote, NumModes
};

// Modulation sources
enum class ModSource : int {
    LFO1 = 0, LFO2, LFO3, LFO4,
    Env1, Env2, Env3, Env4,
    Velocity, ModWheel, Aftertouch, KeyPosition, Random,
    Macro1, Macro2, Macro3, Macro4,
    NumSources
};

// Modulation destinations
enum class ModDest : int {
    // Oscillator destinations (4 osc x 4 destinations = 16)
    Osc1Level = 0, Osc1Pan, Osc1WTPos, Osc1Fine,
    Osc2Level, Osc2Pan, Osc2WTPos, Osc2Fine,
    Osc3Level, Osc3Pan, Osc3WTPos, Osc3Fine,
    Osc4Level, Osc4Pan, Osc4WTPos, Osc4Fine,
    // Filter destinations (2 filt x 3 = 6)
    Filt1Cutoff, Filt1Reso, Filt1Drive,
    Filt2Cutoff, Filt2Reso, Filt2Drive,
    // LFO rate destinations (4)
    LFO1Rate, LFO2Rate, LFO3Rate, LFO4Rate,
    // Master
    MasterLevel,
    NumDestinations
};

// ============================================================================
// Parameter IDs — Phase 1 legacy (kept for backward compat)
// ============================================================================

namespace param {
    constexpr const char* kBypass     = "bypass";
    constexpr const char* kDryWet     = "drywet";
    constexpr const char* kMasterVol  = "master_vol";

    // Phase 1 osc1 IDs — still used, aliased in the new system
    constexpr const char* kOscLevel   = "osc1_level";
    constexpr const char* kOscTune    = "osc1_tune";
    constexpr const char* kOscFine    = "osc1_fine";
    constexpr const char* kOscWaveform = "osc1_waveform";

    // Phase 1 env1 IDs — still used
    constexpr const char* kEnvAttack  = "env1_attack";
    constexpr const char* kEnvHold    = "env1_hold";
    constexpr const char* kEnvDecay   = "env1_decay";
    constexpr const char* kEnvSustain = "env1_sustain";
    constexpr const char* kEnvRelease = "env1_release";
}

// ============================================================================
// Tempo Sync Note Values
// ============================================================================

struct SyncDivision
{
    const char* name;
    float bars; // length in bars (e.g., 0.25 = 1/4 note at 4/4)
};

constexpr SyncDivision kSyncDivisions[] = {
    { "1/64",  1.0f / 16.0f },
    { "1/32",  1.0f / 8.0f },
    { "1/16T", 1.0f / 6.0f },
    { "1/16",  1.0f / 4.0f },
    { "1/16D", 3.0f / 8.0f },
    { "1/8T",  1.0f / 3.0f },
    { "1/8",   1.0f / 2.0f },
    { "1/8D",  3.0f / 4.0f },
    { "1/4T",  2.0f / 3.0f },
    { "1/4",   1.0f },
    { "1/4D",  3.0f / 2.0f },
    { "1/2T",  4.0f / 3.0f },
    { "1/2",   2.0f },
    { "1/2D",  3.0f },
    { "1 Bar", 4.0f },
    { "2 Bar", 8.0f },
    { "4 Bar", 16.0f },
    { "8 Bar", 32.0f },
};
constexpr int kNumSyncDivisions = sizeof (kSyncDivisions) / sizeof (kSyncDivisions[0]);

} // namespace scenememo
