#pragma once

#include "field/AudioImporter.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>

namespace scenememo {

// Results of source analysis — separation into components.
struct AnalysisResult
{
    juce::AudioBuffer<float> tonalContent;     // pitched/harmonic material
    juce::AudioBuffer<float> transientContent; // percussive/rhythmic hits
    juce::AudioBuffer<float> noiseContent;     // ambience, air, room tone
    std::vector<float> pitchContour;           // fundamental frequency per frame (Hz, 0 = unvoiced)
    std::vector<float> spectralCentroid;       // brightness per frame (Hz)
    std::vector<float> onsetStrength;          // transient strength per frame
    float estimatedBPM = 0.0f;
    double sampleRate = 44100.0;
    int hopSize = 512;
};

// HPSS-based source analysis. Runs on a background thread.
class SourceAnalyzer
{
public:
    SourceAnalyzer();
    ~SourceAnalyzer();

    // Analyze the given audio buffer. Returns analysis result.
    // This is a blocking call — run on a background thread.
    std::unique_ptr<AnalysisResult> analyze (const juce::AudioBuffer<float>& mono, double sampleRate);

private:
    static constexpr int kFFTOrder = 11;   // 2048 samples
    static constexpr int kFFTSize = 1 << kFFTOrder;
    static constexpr int kHopSize = 512;
    static constexpr int kMedianFilterSize = 17; // for HPSS

    juce::dsp::FFT fft { kFFTOrder };
    juce::dsp::WindowingFunction<float> window { kFFTSize,
        juce::dsp::WindowingFunction<float>::hann };

    // HPSS: separate harmonic and percussive components
    void performHPSS (const juce::AudioBuffer<float>& mono, double sampleRate,
                      juce::AudioBuffer<float>& harmonic,
                      juce::AudioBuffer<float>& percussive,
                      juce::AudioBuffer<float>& noise);

    // Pitch tracking using autocorrelation
    void trackPitch (const juce::AudioBuffer<float>& mono, double sampleRate,
                     std::vector<float>& pitchContour);

    // Spectral centroid per frame
    void computeSpectralCentroid (const juce::AudioBuffer<float>& mono, double sampleRate,
                                  std::vector<float>& centroid);

    // Onset detection via spectral flux
    void detectOnsets (const juce::AudioBuffer<float>& mono, double sampleRate,
                       std::vector<float>& onsetStrength);

    // Simple BPM estimation from onset strength
    float estimateBPM (const std::vector<float>& onsetStrength, double sampleRate);

    // 1D median filter
    static float medianFilter (const std::vector<float>& data, int pos, int filterSize);
};

} // namespace scenememo
