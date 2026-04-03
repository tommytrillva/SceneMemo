#include "engine/ParamReader.h"

namespace scenememo {

void ParamReader::cachePointers (juce::AudioProcessorValueTreeState& apvts)
{
    auto get = [&] (const juce::String& id) -> std::atomic<float>*
    {
        auto* p = apvts.getRawParameterValue (id);
        jassert (p != nullptr); // parameter must exist
        return p;
    };

    // Global
    bypass    = get (param::kBypass);
    masterVol = get (param::kMasterVol);
    dryWet    = get (param::kDryWet);

    // Oscillators
    for (int i = 0; i < kNumOscSlots; ++i)
    {
        int idx = i + 1;
        auto id = [idx] (const char* s) { return makeParamId ("osc", idx, s); };

        oscPtrs[i].type         = get (id ("type"));
        oscPtrs[i].octave       = get (id ("octave"));
        oscPtrs[i].wtPos        = get (id ("wt_pos"));
        oscPtrs[i].unison       = get (id ("unison"));
        oscPtrs[i].unisonSpread = get (id ("unison_spread"));
        oscPtrs[i].pan          = get (id ("pan"));

        // Phase 1 compat: osc1 uses original IDs
        if (idx == 1)
        {
            oscPtrs[i].waveform = get (param::kOscWaveform);
            oscPtrs[i].level    = get (param::kOscLevel);
            oscPtrs[i].tune     = get (param::kOscTune);
            oscPtrs[i].fine     = get (param::kOscFine);
        }
        else
        {
            oscPtrs[i].waveform = get (id ("waveform"));
            oscPtrs[i].level    = get (id ("level"));
            oscPtrs[i].tune     = get (id ("tune"));
            oscPtrs[i].fine     = get (id ("fine"));
        }
    }

    // Filters
    for (int i = 0; i < kNumFilters; ++i)
    {
        int idx = i + 1;
        auto id = [idx] (const char* s) { return makeParamId ("filt", idx, s); };

        filtPtrs[i].type     = get (id ("type"));
        filtPtrs[i].cutoff   = get (id ("cutoff"));
        filtPtrs[i].reso     = get (id ("reso"));
        filtPtrs[i].drive    = get (id ("drive"));
        filtPtrs[i].keyTrack = get (id ("keyTrack"));
        filtPtrs[i].envAmt   = get (id ("env_amt"));
    }
    filtRouting = get ("filt_routing");

    // Envelopes
    for (int i = 0; i < kNumEnvelopes; ++i)
    {
        int idx = i + 1;
        auto id = [idx] (const char* s) { return makeParamId ("env", idx, s); };

        // Phase 1 compat: env1 uses original IDs
        if (idx == 1)
        {
            envPtrs[i].attack  = get (param::kEnvAttack);
            envPtrs[i].hold    = get (param::kEnvHold);
            envPtrs[i].decay   = get (param::kEnvDecay);
            envPtrs[i].sustain = get (param::kEnvSustain);
            envPtrs[i].release = get (param::kEnvRelease);
        }
        else
        {
            envPtrs[i].attack  = get (id ("attack"));
            envPtrs[i].hold    = get (id ("hold"));
            envPtrs[i].decay   = get (id ("decay"));
            envPtrs[i].sustain = get (id ("sustain"));
            envPtrs[i].release = get (id ("release"));
        }

        envPtrs[i].attackCurve  = get (id ("attack_curve"));
        envPtrs[i].decayCurve   = get (id ("decay_curve"));
        envPtrs[i].releaseCurve = get (id ("release_curve"));
    }

    // LFOs
    for (int i = 0; i < kNumLFOs; ++i)
    {
        int idx = i + 1;
        auto id = [idx] (const char* s) { return makeParamId ("lfo", idx, s); };

        lfoPtrs[i].shape     = get (id ("shape"));
        lfoPtrs[i].rate      = get (id ("rate"));
        lfoPtrs[i].sync      = get (id ("sync"));
        lfoPtrs[i].syncRate  = get (id ("sync_rate"));
        lfoPtrs[i].phase     = get (id ("phase"));
        lfoPtrs[i].fadeIn    = get (id ("fadein"));
        lfoPtrs[i].retrigger = get (id ("retrigger"));
        lfoPtrs[i].humanize  = get (id ("humanize"));
    }

    // Mod matrix
    for (int i = 0; i < kMaxModSlots; ++i)
    {
        int idx = i + 1;
        auto id = [idx] (const char* s) { return makeParamId ("mod", idx, s); };

        modPtrs[i].source = get (id ("source"));
        modPtrs[i].dest   = get (id ("dest"));
        modPtrs[i].depth  = get (id ("depth"));
    }

    // Macros
    for (int i = 0; i < kNumMacros; ++i)
        macroPtrs[i] = get ("macro" + juce::String (i + 1));
}

void ParamReader::readAll (EngineParams& out) const
{
    out.bypass    = bypass->load() >= 0.5f;
    out.masterVol = masterVol->load();
    out.dryWet    = dryWet->load();

    for (int i = 0; i < kNumOscSlots; ++i)
    {
        out.osc[i].type         = static_cast<int> (oscPtrs[i].type->load());
        out.osc[i].waveform     = static_cast<int> (oscPtrs[i].waveform->load());
        out.osc[i].level        = oscPtrs[i].level->load();
        out.osc[i].pan          = oscPtrs[i].pan->load();
        out.osc[i].tune         = oscPtrs[i].tune->load();
        out.osc[i].fine         = oscPtrs[i].fine->load();
        out.osc[i].octave       = oscPtrs[i].octave->load();
        out.osc[i].wtPos        = oscPtrs[i].wtPos->load();
        out.osc[i].unison       = static_cast<int> (oscPtrs[i].unison->load());
        out.osc[i].unisonSpread = oscPtrs[i].unisonSpread->load();
    }

    for (int i = 0; i < kNumFilters; ++i)
    {
        out.filt[i].type     = static_cast<int> (filtPtrs[i].type->load());
        out.filt[i].cutoff   = filtPtrs[i].cutoff->load();
        out.filt[i].reso     = filtPtrs[i].reso->load();
        out.filt[i].drive    = filtPtrs[i].drive->load();
        out.filt[i].keyTrack = filtPtrs[i].keyTrack->load();
        out.filt[i].envAmt   = filtPtrs[i].envAmt->load();
    }
    out.filtRouting = static_cast<int> (filtRouting->load());

    for (int i = 0; i < kNumEnvelopes; ++i)
    {
        out.env[i].attack       = envPtrs[i].attack->load();
        out.env[i].hold         = envPtrs[i].hold->load();
        out.env[i].decay        = envPtrs[i].decay->load();
        out.env[i].sustain      = envPtrs[i].sustain->load();
        out.env[i].release      = envPtrs[i].release->load();
        out.env[i].attackCurve  = envPtrs[i].attackCurve->load();
        out.env[i].decayCurve   = envPtrs[i].decayCurve->load();
        out.env[i].releaseCurve = envPtrs[i].releaseCurve->load();
    }

    for (int i = 0; i < kNumLFOs; ++i)
    {
        out.lfo[i].shape     = static_cast<int> (lfoPtrs[i].shape->load());
        out.lfo[i].rate      = lfoPtrs[i].rate->load();
        out.lfo[i].sync      = lfoPtrs[i].sync->load() >= 0.5f;
        out.lfo[i].syncRate  = static_cast<int> (lfoPtrs[i].syncRate->load());
        out.lfo[i].phase     = lfoPtrs[i].phase->load();
        out.lfo[i].fadeIn    = lfoPtrs[i].fadeIn->load();
        out.lfo[i].retrigger = static_cast<int> (lfoPtrs[i].retrigger->load());
        out.lfo[i].humanize  = lfoPtrs[i].humanize->load();
    }

    for (int i = 0; i < kMaxModSlots; ++i)
    {
        out.mod[i].source = static_cast<int> (modPtrs[i].source->load());
        out.mod[i].dest   = static_cast<int> (modPtrs[i].dest->load());
        out.mod[i].depth  = modPtrs[i].depth->load();
    }

    for (int i = 0; i < kNumMacros; ++i)
        out.macro[i] = macroPtrs[i]->load();
}

} // namespace scenememo
