#pragma once

#include <array>
#include <cstdint>

namespace scenememo {

// Texture event types for the Micro-Sequencer
enum class TextureEvent : int
{
    Nothing = 0,
    ReverseSwell,
    FilterSweep,
    SpectralFreeze,
    HarmonicShift,
    GrainDensityBurst,
    BitCrushSpike,
    NoiseBurst,
    VolumeSwell,
    StereoWidthShift,
    NumEvents
};

struct MicroStep
{
    TextureEvent event = TextureEvent::Nothing;
    float intensity = 0.5f;    // 0-1
    float duration = 0.25f;    // beats
    float probability = 1.0f;  // 0-1
};

struct MicroSequencerParams
{
    int patternLength = 16;     // 1-16
    int stepResolution = 2;     // 0=1/4, 1=1/8, 2=1/16, 3=1/32
    float swing = 0.0f;        // 0-1
    bool enabled = false;
};

class MicroSequencer
{
public:
    MicroSequencer();

    void prepareToPlay (float sampleRate);
    void reset();

    // Advance the sequencer and return any triggered event.
    // Returns the event at the current step, or Nothing if no trigger.
    // Call once per audio block with current host position.
    struct TriggeredEvent
    {
        TextureEvent event = TextureEvent::Nothing;
        float intensity = 0.0f;
        bool triggered = false;
    };

    TriggeredEvent processBlock (int numSamples, float bpm, double ppqPosition,
                                  const MicroSequencerParams& params);

    // Pattern editing
    void setStep (int patternSlot, int stepIndex, const MicroStep& step);
    MicroStep getStep (int patternSlot, int stepIndex) const;
    void setActivePattern (int slot) { activePattern = slot; }

    static constexpr int kMaxSteps = 16;
    static constexpr int kNumPatterns = 4;

private:
    float sampleRate = 44100.0f;
    int activePattern = 0;
    int lastStep = -1;

    std::array<std::array<MicroStep, kMaxSteps>, kNumPatterns> patterns;

    uint32_t rngState = 54321;
    float nextRng();
};

} // namespace scenememo
