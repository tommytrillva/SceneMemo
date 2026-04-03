#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

namespace scenememo {

// Resynthesis modes for frozen spectrum
enum class ResynthesisMode { Additive, Granular, Wavetable };

// Parameters for spectral manipulation
struct SpectralParams
{
    float spectralShift = 0.0f;  // semitones (-24 to +24)
    float spectralTilt = 0.0f;   // -1 to +1 (darken/brighten)
    float spectralBlur = 0.0f;   // 0-1 (frequency bin smearing)
    ResynthesisMode resynthMode = ResynthesisMode::Additive;
};

class SpectralFreeze
{
public:
    SpectralFreeze();

    void prepareToPlay (float sampleRate);
    void reset();

    // Capture a single FFT frame from the source at the given sample position.
    void freeze (const juce::AudioBuffer<float>& source, int position, int fftSizeChoice = 2);

    // Unfreeze — stop sustained tone
    void unfreeze();

    bool isFrozen() const { return frozen; }

    // Render the frozen spectrum as a sustained tone (additive resynthesis).
    void renderBlock (float* outputL, float* outputR, int numSamples,
                      const SpectralParams& params, float pitchShiftSemitones);

    // Get the frozen magnitude spectrum for visualization
    const std::vector<float>& getFrozenMagnitudes() const { return frozenMagnitudes; }

private:
    static constexpr int kMaxFFTOrder = 14; // 16384
    static constexpr int kMaxFFTSize = 1 << kMaxFFTOrder;
    static constexpr int kMaxBins = kMaxFFTSize / 2 + 1;

    float sampleRate = 44100.0f;
    bool frozen = false;
    int currentFFTSize = 4096;

    // Frozen spectral data
    std::vector<float> frozenMagnitudes;
    std::vector<float> frozenPhases;

    // Additive resynthesis: per-partial oscillator phases
    std::vector<float> partialPhases;

    // Apply spectral manipulation to a copy of the frozen spectrum
    void applySpectralManipulation (std::vector<float>& mags, const SpectralParams& params) const;
};

} // namespace scenememo
