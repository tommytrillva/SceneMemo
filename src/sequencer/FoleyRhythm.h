#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <memory>

namespace scenememo {

// Per-slot foley sample
struct FoleySlot
{
    juce::AudioBuffer<float> buffer;
    int numSamples = 0;
    float level = 0.8f;
    float pan = 0.0f;
    float pitch = 0.0f;     // semitones (-12 to +12)
    int startSample = 0;
    int endSample = 0;
    bool oneShot = true;
    bool loaded = false;
};

struct FoleyStep
{
    int slotIndex = -1;     // -1 = inactive
    float velocity = 0.8f;  // 0-1
    float probability = 1.0f;
    float pitchOffset = 0.0f; // semitones
};

struct FoleyRhythmParams
{
    int patternLength = 16;
    int stepResolution = 2;  // 0=1/4, 1=1/8, 2=1/16, 3=1/32
    float swing = 0.0f;
    float humanizeTiming = 0.0f; // ms jitter (0-50)
    float humanizeVelocity = 0.0f; // 0-1 jitter
    bool enabled = false;
};

class FoleyRhythm
{
public:
    FoleyRhythm();

    void prepareToPlay (float sampleRate);
    void reset();

    // Load a sample into a slot
    void loadSlot (int slotIndex, const juce::AudioBuffer<float>& audio, int numSamples);
    void clearSlot (int slotIndex);

    // Advance the sequencer and render active samples into output (additive)
    void processBlock (float* outputL, float* outputR, int numSamples,
                       float bpm, double ppqPosition, const FoleyRhythmParams& params);

    // Pattern editing
    void setStep (int patternSlot, int stepIndex, const FoleyStep& step);
    FoleyStep getStep (int patternSlot, int stepIndex) const;
    void setActivePattern (int slot) { activePattern = slot; }

    static constexpr int kNumSlots = 8;
    static constexpr int kMaxSteps = 16;
    static constexpr int kNumPatterns = 8;

private:
    float sampleRate = 44100.0f;
    int activePattern = 0;
    int lastStep = -1;

    std::array<FoleySlot, kNumSlots> slots;
    std::array<std::array<FoleyStep, kMaxSteps>, kNumPatterns> patterns;

    // Active playback voices (one per slot for simplicity)
    struct PlaybackVoice
    {
        bool active = false;
        int slotIndex = -1;
        float readPos = 0.0f;
        float pitchRatio = 1.0f;
        float velocity = 1.0f;
        float panL = 1.0f;
        float panR = 1.0f;
    };
    std::array<PlaybackVoice, kNumSlots> playbackVoices;

    uint32_t rngState = 99999;
    float nextRng();
    void triggerSlot (int slotIndex, float velocity, float pitchOffset);
};

} // namespace scenememo
