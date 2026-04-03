#pragma once

#include "util/Constants.h"
#include <array>

namespace scenememo {

// Per-voice modulation state. Plain data, no methods needed.
struct VoiceModState
{
    // Source values filled each sub-block from LFOs, envelopes, etc.
    std::array<float, kMaxModSources> sourceValues {};

    // Destination offsets accumulated by the mod matrix
    std::array<float, kMaxModDestinations> destOffsets {};

    // Per-voice MIDI / performance state
    float velocity = 0.0f;
    float keyPosition = 0.0f;   // normalized 0-1 across keyboard (note/127)
    float modWheel = 0.0f;
    float aftertouch = 0.0f;
    float randomValue = 0.0f;   // generated at noteOn

    void clear()
    {
        sourceValues.fill (0.0f);
        destOffsets.fill (0.0f);
    }
};

} // namespace scenememo
