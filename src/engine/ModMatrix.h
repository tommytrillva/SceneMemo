#pragma once

#include "util/Constants.h"
#include "engine/VoiceModState.h"
#include "engine/ParamReader.h"
#include <array>

namespace scenememo {

class ModMatrix
{
public:
    ModMatrix() = default;

    // Evaluate all mod slots: reads sourceValues, writes destOffsets.
    void evaluate (VoiceModState& state, const EngineParams& params) const;

    // Convenience: get a destination offset after evaluation
    static float getDestOffset (const VoiceModState& state, ModDest dest)
    {
        int idx = static_cast<int> (dest);
        if (idx >= 0 && idx < kMaxModDestinations)
            return state.destOffsets[static_cast<size_t> (idx)];
        return 0.0f;
    }

    // Fill source values from envelopes, LFOs, and performance state
    static void fillSources (VoiceModState& state,
                             const float* envValues, int numEnvs,
                             const float* lfoValues, int numLfos,
                             const float* macroValues, int numMacros);
};

} // namespace scenememo
