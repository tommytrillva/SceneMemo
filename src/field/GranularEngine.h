#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <cstdint>

namespace scenememo {

// Grain window shapes
enum class GrainWindow { Hanning, Hamming, Gaussian, Triangular, Rectangular };

// Playback modes for the granular engine
enum class GranularMode { Scrub, Scan, Freeze, Scatter, Sequence };

// Single grain state
struct Grain
{
    bool active = false;
    int sourcePos = 0;       // start position in source buffer (samples)
    int grainLength = 0;     // length in samples
    int currentSample = 0;   // playback position within grain
    float pitchRatio = 1.0f; // playback speed (1.0 = original pitch)
    float readPos = 0.0f;    // fractional read position for pitch shifting
    float panL = 1.0f;
    float panR = 1.0f;
    float amplitude = 1.0f;
    GrainWindow window = GrainWindow::Hanning;
};

// Parameters controlling the granular engine
struct GranularParams
{
    // Core grain parameters
    float grainSize = 50.0f;     // ms (1-500)
    float grainDensity = 20.0f;  // grains/second (1-100)
    float grainPosition = 0.5f;  // 0-1 through source file
    float scanSpeed = 0.5f;      // normalized rate for Scan mode
    float pitchShift = 0.0f;     // semitones (-48 to +48)

    // Randomization
    float positionJitter = 0.0f;  // 0-1
    float pitchJitter = 0.0f;     // 0-1 (maps to cents)
    float levelJitter = 0.0f;     // 0-1
    float panJitter = 0.0f;       // 0-1

    // Mode
    GranularMode mode = GranularMode::Scrub;
    GrainWindow window = GrainWindow::Hanning;

    // Pitch quantize
    bool pitchQuantize = false;
    int scaleRoot = 0;            // 0=C, 1=C#, etc.
    int scaleType = 0;            // index into scale table
};

class GranularEngine
{
public:
    GranularEngine();

    void prepareToPlay (float sampleRate);
    void setSourceBuffer (const juce::AudioBuffer<float>* source, double sourceSampleRate);
    void reset();

    // Render granular output into the stereo buffer (additive).
    // midiNoteForPitch: -1 = use pitchShift param, >=0 = MIDI note-driven pitch
    void renderBlock (float* outputL, float* outputR, int numSamples,
                      const GranularParams& params, int midiNoteForPitch = -1);

    // Trigger a grain manually (for sequenced mode)
    void triggerGrain (const GranularParams& params);

    bool hasSource() const { return sourceBuffer != nullptr && sourceBuffer->getNumSamples() > 0; }

private:
    static constexpr int kMaxGrains = 128;

    float sampleRate = 44100.0f;
    const juce::AudioBuffer<float>* sourceBuffer = nullptr;
    double sourceSampleRate = 44100.0;

    std::array<Grain, kMaxGrains> grains;
    float grainTimer = 0.0f; // counts down to next grain spawn

    // Scan state
    float scanPosition = 0.0f;

    // RNG
    uint32_t rngState = 42;

    float nextRng();
    void spawnGrain (const GranularParams& params, int midiNoteForPitch);
    float readSourceSample (float position) const;
    static float grainWindowValue (GrainWindow window, float phase);
    float quantizePitch (float freqHz, int scaleRoot, int scaleType) const;
};

} // namespace scenememo
