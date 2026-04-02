#pragma once

#include <vector>
#include <memory>
#include <cmath>
#include "util/Constants.h"

namespace scenememo {

// Immutable wavetable data with mip-map levels for anti-aliased playback.
// Shared across voices via shared_ptr — no audio-thread allocation.
class Wavetable
{
public:
    struct MipLevel
    {
        std::vector<float> samples;
        int size = 0;
    };

    // Construct from raw single-cycle waveform. Builds all mip levels.
    Wavetable (const std::vector<float>& rawSamples);

    const MipLevel& getMipLevel (int level) const;
    int getNumMipLevels() const { return static_cast<int> (mipLevels.size()); }

private:
    std::vector<MipLevel> mipLevels;

    void buildMipLevels (const std::vector<float>& baseTable);
};

// Per-voice wavetable oscillator with phase accumulator and mip-map selection.
class WavetableOsc
{
public:
    WavetableOsc() = default;

    void setWavetable (std::shared_ptr<const Wavetable> wt);
    void setFrequency (float freqHz, float sampleRate);
    void reset();

    float processSample();

private:
    std::shared_ptr<const Wavetable> wavetable;
    float phase = 0.0f;
    float phaseIncrement = 0.0f;
    float currentFrequency = 440.0f;
    float sampleRate = 44100.0f;

    int selectMipLevel() const;
    static float interpolateSample (const Wavetable::MipLevel& level, float normalizedPhase);
};

// Factory: builds the 4 basic waveforms (Sine, Saw, Square, Triangle)
// with full mip-map chains. Call once at init, NOT on audio thread.
struct WavetableFactory
{
    static std::shared_ptr<const Wavetable> createSine (int size = kDefaultWavetableSize);
    static std::shared_ptr<const Wavetable> createSaw (int size = kDefaultWavetableSize);
    static std::shared_ptr<const Wavetable> createSquare (int size = kDefaultWavetableSize);
    static std::shared_ptr<const Wavetable> createTriangle (int size = kDefaultWavetableSize);
};

} // namespace scenememo
