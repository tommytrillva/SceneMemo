#include "engine/ParameterLayout.h"
#include "util/Constants.h"

namespace scenememo {
namespace params {

// Sync rate choice names
static juce::StringArray getSyncRateNames()
{
    juce::StringArray names;
    for (int i = 0; i < kNumSyncDivisions; ++i)
        names.add (kSyncDivisions[i].name);
    return names;
}

void addGlobalParams (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& p)
{
    p.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { param::kBypass, 1 }, "Bypass", false));

    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { param::kMasterVol, 1 }, "Master Volume",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.0f, 0.5f), 0.8f));

    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { param::kDryWet, 1 }, "Dry/Wet",
        juce::NormalisableRange<float> (0.0f, 1.0f), 1.0f));
}

void addOscParams (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& p, int i)
{
    auto id = [i] (const char* s) { return makeParamId ("osc", i, s); };
    auto name = [i] (const char* s) { return "Osc " + juce::String (i) + " " + s; };

    // Type selector (Wavetable / VA / Noise / Sub)
    p.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { id ("type"), 1 }, name ("Type"),
        juce::StringArray { "Wavetable", "Virtual Analog", "Noise", "Sub" }, 0));

    // Waveform (context-dependent: WT shapes for Wavetable, VA shapes for VA, etc.)
    // Phase 1 compat: osc1_waveform stays as the ID for osc1
    juce::String waveformId = (i == 1) ? param::kOscWaveform : id ("waveform");
    p.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { waveformId, 1 }, name ("Waveform"),
        juce::StringArray { "Sine", "Saw", "Square", "Triangle", "Pulse" }, 0));

    // Level (0-1) — Phase 1 compat: osc1_level stays as-is
    juce::String levelId = (i == 1) ? param::kOscLevel : id ("level");
    float defaultLevel = (i == 1) ? 0.8f : 0.0f; // osc2-4 default to off
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { levelId, 1 }, name ("Level"),
        juce::NormalisableRange<float> (0.0f, 1.0f), defaultLevel));

    // Pan (-1 to +1)
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id ("pan"), 1 }, name ("Pan"),
        juce::NormalisableRange<float> (-1.0f, 1.0f), 0.0f));

    // Coarse tune (-24 to +24 semitones) — Phase 1 compat: osc1_tune
    juce::String tuneId = (i == 1) ? param::kOscTune : id ("tune");
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { tuneId, 1 }, name ("Tune"),
        juce::NormalisableRange<float> (-24.0f, 24.0f, 1.0f), 0.0f));

    // Fine tune (-100 to +100 cents) — Phase 1 compat: osc1_fine
    juce::String fineId = (i == 1) ? param::kOscFine : id ("fine");
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { fineId, 1 }, name ("Fine"),
        juce::NormalisableRange<float> (-100.0f, 100.0f, 0.1f), 0.0f));

    // Octave (-2 to +2)
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id ("octave"), 1 }, name ("Octave"),
        juce::NormalisableRange<float> (-2.0f, 2.0f, 1.0f), 0.0f));

    // Wavetable position (0-1, modulatable)
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id ("wt_pos"), 1 }, name ("WT Position"),
        juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));

    // Unison voices (1-8)
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id ("unison"), 1 }, name ("Unison"),
        juce::NormalisableRange<float> (1.0f, 8.0f, 1.0f), 1.0f));

    // Unison spread (0-1)
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id ("unison_spread"), 1 }, name ("Unison Spread"),
        juce::NormalisableRange<float> (0.0f, 1.0f), 0.5f));
}

void addFilterParams (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& p, int i)
{
    auto id = [i] (const char* s) { return makeParamId ("filt", i, s); };
    auto name = [i] (const char* s) { return "Filter " + juce::String (i) + " " + s; };

    // Type
    p.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { id ("type"), 1 }, name ("Type"),
        juce::StringArray { "LP12", "LP24", "LP36", "HP12", "HP24", "BP", "Notch", "Comb", "Formant" }, 1));

    // Cutoff (20Hz - 20kHz, log skew)
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id ("cutoff"), 1 }, name ("Cutoff"),
        juce::NormalisableRange<float> (20.0f, 20000.0f, 0.0f, 0.3f), 20000.0f));

    // Resonance (0-1)
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id ("reso"), 1 }, name ("Resonance"),
        juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));

    // Drive (0-1)
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id ("drive"), 1 }, name ("Drive"),
        juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));

    // Key tracking (0-1)
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id ("keytrack"), 1 }, name ("Key Track"),
        juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));

    // Envelope amount (-1 to +1)
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id ("env_amt"), 1 }, name ("Env Amount"),
        juce::NormalisableRange<float> (-1.0f, 1.0f), 0.0f));
}

void addFilterRoutingParam (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& p)
{
    p.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "filt_routing", 1 }, "Filter Routing",
        juce::StringArray { "Series", "Parallel", "Split" }, 0));
}

void addEnvParams (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& p, int i)
{
    auto id = [i] (const char* s) { return makeParamId ("env", i, s); };
    auto name = [i] (const char* s) { return "Env " + juce::String (i) + " " + s; };

    // Phase 1 compat: env1 uses the original IDs
    juce::String attackId  = (i == 1) ? juce::String (param::kEnvAttack)  : id ("attack");
    juce::String holdId    = (i == 1) ? juce::String (param::kEnvHold)    : id ("hold");
    juce::String decayId   = (i == 1) ? juce::String (param::kEnvDecay)   : id ("decay");
    juce::String sustainId = (i == 1) ? juce::String (param::kEnvSustain) : id ("sustain");
    juce::String releaseId = (i == 1) ? juce::String (param::kEnvRelease) : id ("release");

    float defaultSustain = (i == 1) ? kDefaultSustain : 0.0f;

    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { attackId, 1 }, name ("Attack"),
        juce::NormalisableRange<float> (kMinEnvTime, kMaxAttack, 0.0f, 0.4f), kDefaultAttack));

    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { holdId, 1 }, name ("Hold"),
        juce::NormalisableRange<float> (0.0f, kMaxHold, 0.0f, 0.5f), kDefaultHold));

    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { decayId, 1 }, name ("Decay"),
        juce::NormalisableRange<float> (kMinEnvTime, kMaxDecay, 0.0f, 0.4f), kDefaultDecay));

    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { sustainId, 1 }, name ("Sustain"),
        juce::NormalisableRange<float> (0.0f, 1.0f), defaultSustain));

    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { releaseId, 1 }, name ("Release"),
        juce::NormalisableRange<float> (kMinEnvTime, kMaxRelease, 0.0f, 0.4f), kDefaultRelease));

    // Curve shapes (new in Phase 2): -1 concave, 0 linear, +1 convex
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id ("attack_curve"), 1 }, name ("Attack Curve"),
        juce::NormalisableRange<float> (-1.0f, 1.0f), 0.0f));

    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id ("decay_curve"), 1 }, name ("Decay Curve"),
        juce::NormalisableRange<float> (-1.0f, 1.0f), 0.0f));

    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id ("release_curve"), 1 }, name ("Release Curve"),
        juce::NormalisableRange<float> (-1.0f, 1.0f), 0.0f));
}

void addLfoParams (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& p, int i)
{
    auto id = [i] (const char* s) { return makeParamId ("lfo", i, s); };
    auto name = [i] (const char* s) { return "LFO " + juce::String (i) + " " + s; };

    // Shape
    p.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { id ("shape"), 1 }, name ("Shape"),
        juce::StringArray { "Sine", "Triangle", "Saw Up", "Saw Down", "Square",
                            "S&H", "Random Smooth", "Custom 32" }, 0));

    // Rate (free-running, Hz)
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id ("rate"), 1 }, name ("Rate"),
        juce::NormalisableRange<float> (0.01f, 50.0f, 0.0f, 0.35f), 1.0f));

    // Tempo sync on/off
    p.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { id ("sync"), 1 }, name ("Sync"), false));

    // Sync rate (index into kSyncDivisions)
    p.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { id ("sync_rate"), 1 }, name ("Sync Rate"),
        getSyncRateNames(), 9)); // default: 1/4 note

    // Phase offset (0-360 degrees, stored as 0-1)
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id ("phase"), 1 }, name ("Phase"),
        juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));

    // Fade-in time (seconds)
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id ("fadein"), 1 }, name ("Fade In"),
        juce::NormalisableRange<float> (0.0f, 10.0f, 0.0f, 0.4f), 0.0f));

    // Retrigger mode
    p.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { id ("retrigger"), 1 }, name ("Retrigger"),
        juce::StringArray { "Free", "Note On", "First Note" }, 0));

    // Humanize (0-1)
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id ("humanize"), 1 }, name ("Humanize"),
        juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
}

void addModSlotParams (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& p, int i)
{
    auto id = [i] (const char* s) { return makeParamId ("mod", i, s); };
    auto name = [i] (const char* s) { return "Mod " + juce::String (i) + " " + s; };

    // Source
    juce::StringArray sourceNames {
        "LFO 1", "LFO 2", "LFO 3", "LFO 4",
        "Env 1", "Env 2", "Env 3", "Env 4",
        "Velocity", "Mod Wheel", "Aftertouch", "Key Position", "Random",
        "Macro 1", "Macro 2", "Macro 3", "Macro 4"
    };
    p.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { id ("source"), 1 }, name ("Source"),
        sourceNames, 0));

    // Destination
    juce::StringArray destNames;
    for (int osc = 1; osc <= 4; ++osc)
    {
        auto prefix = "Osc " + juce::String (osc) + " ";
        destNames.add (prefix + "Level");
        destNames.add (prefix + "Pan");
        destNames.add (prefix + "WT Pos");
        destNames.add (prefix + "Fine");
    }
    for (int filt = 1; filt <= 2; ++filt)
    {
        auto prefix = "Filter " + juce::String (filt) + " ";
        destNames.add (prefix + "Cutoff");
        destNames.add (prefix + "Reso");
        destNames.add (prefix + "Drive");
    }
    destNames.add ("LFO 1 Rate");
    destNames.add ("LFO 2 Rate");
    destNames.add ("LFO 3 Rate");
    destNames.add ("LFO 4 Rate");
    destNames.add ("Master Level");

    p.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { id ("dest"), 1 }, name ("Dest"),
        destNames, 0));

    // Depth (bipolar -1 to +1)
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id ("depth"), 1 }, name ("Depth"),
        juce::NormalisableRange<float> (-1.0f, 1.0f), 0.0f));
}

void addMacroParams (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& p)
{
    for (int i = 1; i <= kNumMacros; ++i)
    {
        auto idStr = "macro" + juce::String (i);
        auto nameStr = "Macro " + juce::String (i);
        p.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { idStr, 1 }, nameStr,
            juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout createFullLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    addGlobalParams (p);

    for (int i = 1; i <= kNumOscSlots; ++i)
        addOscParams (p, i);

    for (int i = 1; i <= kNumFilters; ++i)
        addFilterParams (p, i);
    addFilterRoutingParam (p);

    for (int i = 1; i <= kNumEnvelopes; ++i)
        addEnvParams (p, i);

    for (int i = 1; i <= kNumLFOs; ++i)
        addLfoParams (p, i);

    for (int i = 1; i <= kMaxModSlots; ++i)
        addModSlotParams (p, i);

    addMacroParams (p);

    return { p.begin(), p.end() };
}

} // namespace params
} // namespace scenememo
