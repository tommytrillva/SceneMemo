#include "engine/ModMatrix.h"
#include <cmath>

namespace scenememo {

void ModMatrix::evaluate (VoiceModState& state, const EngineParams& params) const
{
    // Clear destination offsets
    state.destOffsets.fill (0.0f);

    // Sum all active mod slots
    for (int i = 0; i < kMaxModSlots; ++i)
    {
        float depth = params.mod[i].depth;
        if (std::abs (depth) < 0.0001f)
            continue; // skip inactive slots

        int srcIdx = params.mod[i].source;
        int dstIdx = params.mod[i].dest;

        if (srcIdx < 0 || srcIdx >= kMaxModSources)
            continue;
        if (dstIdx < 0 || dstIdx >= kMaxModDestinations)
            continue;

        float srcVal = state.sourceValues[static_cast<size_t> (srcIdx)];
        state.destOffsets[static_cast<size_t> (dstIdx)] += srcVal * depth;
    }
}

void ModMatrix::fillSources (VoiceModState& state,
                              const float* envValues, int numEnvs,
                              const float* lfoValues, int numLfos,
                              const float* macroValues, int numMacros)
{
    state.sourceValues.fill (0.0f);

    // LFO 1-4 → source indices 0-3
    for (int i = 0; i < numLfos && i < 4; ++i)
        state.sourceValues[static_cast<size_t> (static_cast<int> (ModSource::LFO1) + i)] = lfoValues[i];

    // Env 1-4 → source indices 4-7
    for (int i = 0; i < numEnvs && i < 4; ++i)
        state.sourceValues[static_cast<size_t> (static_cast<int> (ModSource::Env1) + i)] = envValues[i];

    // Performance sources
    state.sourceValues[static_cast<size_t> (ModSource::Velocity)]    = state.velocity;
    state.sourceValues[static_cast<size_t> (ModSource::ModWheel)]    = state.modWheel;
    state.sourceValues[static_cast<size_t> (ModSource::Aftertouch)]  = state.aftertouch;
    state.sourceValues[static_cast<size_t> (ModSource::KeyPosition)] = state.keyPosition;
    state.sourceValues[static_cast<size_t> (ModSource::Random)]      = state.randomValue;

    // Macros 1-4
    for (int i = 0; i < numMacros && i < 4; ++i)
        state.sourceValues[static_cast<size_t> (static_cast<int> (ModSource::Macro1) + i)] = macroValues[i];
}

} // namespace scenememo
