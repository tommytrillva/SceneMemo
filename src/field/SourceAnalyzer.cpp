#include "field/SourceAnalyzer.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace scenememo {

SourceAnalyzer::SourceAnalyzer() = default;
SourceAnalyzer::~SourceAnalyzer() = default;

std::unique_ptr<AnalysisResult> SourceAnalyzer::analyze (const juce::AudioBuffer<float>& mono, double sampleRate)
{
    auto result = std::make_unique<AnalysisResult>();
    result->sampleRate = sampleRate;
    result->hopSize = kHopSize;

    // HPSS separation
    performHPSS (mono, sampleRate, result->tonalContent, result->transientContent, result->noiseContent);

    // Pitch tracking on tonal content
    trackPitch (result->tonalContent, sampleRate, result->pitchContour);

    // Spectral centroid
    computeSpectralCentroid (mono, sampleRate, result->spectralCentroid);

    // Onset detection
    detectOnsets (mono, sampleRate, result->onsetStrength);

    // BPM estimation
    result->estimatedBPM = estimateBPM (result->onsetStrength, sampleRate);

    return result;
}

void SourceAnalyzer::performHPSS (const juce::AudioBuffer<float>& mono, double /*sampleRate*/,
                                   juce::AudioBuffer<float>& harmonic,
                                   juce::AudioBuffer<float>& percussive,
                                   juce::AudioBuffer<float>& noise)
{
    const int numSamples = mono.getNumSamples();
    const int numFrames = (numSamples - kFFTSize) / kHopSize + 1;
    if (numFrames <= 0)
    {
        harmonic = mono;
        percussive.setSize (1, numSamples);
        percussive.clear();
        noise.setSize (1, numSamples);
        noise.clear();
        return;
    }

    const int numBins = kFFTSize / 2 + 1;
    const float* input = mono.getReadPointer (0);

    // Compute magnitude spectrogram
    std::vector<std::vector<float>> magSpec (numFrames, std::vector<float> (numBins, 0.0f));
    std::vector<std::vector<float>> phaseSpec (numFrames, std::vector<float> (numBins, 0.0f));

    std::vector<float> fftBuffer (kFFTSize * 2, 0.0f);

    for (int frame = 0; frame < numFrames; ++frame)
    {
        int offset = frame * kHopSize;

        // Copy and window
        std::fill (fftBuffer.begin(), fftBuffer.end(), 0.0f);
        for (int i = 0; i < kFFTSize && (offset + i) < numSamples; ++i)
            fftBuffer[i] = input[offset + i];

        window.multiplyWithWindowingTable (fftBuffer.data(), kFFTSize);

        // Forward FFT
        fft.performRealOnlyForwardTransform (fftBuffer.data());

        // Extract magnitude and phase
        for (int bin = 0; bin < numBins; ++bin)
        {
            float re = fftBuffer[bin * 2];
            float im = fftBuffer[bin * 2 + 1];
            magSpec[frame][bin] = std::sqrt (re * re + im * im);
            phaseSpec[frame][bin] = std::atan2 (im, re);
        }
    }

    // Apply median filtering: horizontal (time) for harmonic, vertical (frequency) for percussive
    std::vector<std::vector<float>> harmonicMask (numFrames, std::vector<float> (numBins, 0.0f));
    std::vector<std::vector<float>> percussiveMask (numFrames, std::vector<float> (numBins, 0.0f));

    for (int frame = 0; frame < numFrames; ++frame)
    {
        for (int bin = 0; bin < numBins; ++bin)
        {
            // Horizontal median (along time axis) → harmonic estimate
            std::vector<float> timeSlice (numFrames);
            for (int f = 0; f < numFrames; ++f)
                timeSlice[f] = magSpec[f][bin];
            float harmonicEst = medianFilter (timeSlice, frame, kMedianFilterSize);

            // Vertical median (along frequency axis) → percussive estimate
            float percussiveEst = medianFilter (magSpec[frame], bin, kMedianFilterSize);

            // Soft mask based on Wiener filtering
            float total = harmonicEst + percussiveEst + 1e-10f;
            harmonicMask[frame][bin] = (harmonicEst * harmonicEst) / (harmonicEst * harmonicEst + percussiveEst * percussiveEst + 1e-10f);
            percussiveMask[frame][bin] = 1.0f - harmonicMask[frame][bin];
        }
    }

    // Reconstruct via inverse FFT with overlap-add
    harmonic.setSize (1, numSamples);
    harmonic.clear();
    percussive.setSize (1, numSamples);
    percussive.clear();

    float* harmonicOut = harmonic.getWritePointer (0);
    float* percussiveOut = percussive.getWritePointer (0);

    for (int frame = 0; frame < numFrames; ++frame)
    {
        // Reconstruct harmonic frame
        std::vector<float> harmonicFFT (kFFTSize * 2, 0.0f);
        std::vector<float> percussiveFFT (kFFTSize * 2, 0.0f);

        for (int bin = 0; bin < numBins; ++bin)
        {
            float mag = magSpec[frame][bin];
            float phase = phaseSpec[frame][bin];

            float hMag = mag * harmonicMask[frame][bin];
            float pMag = mag * percussiveMask[frame][bin];

            harmonicFFT[bin * 2]     = hMag * std::cos (phase);
            harmonicFFT[bin * 2 + 1] = hMag * std::sin (phase);

            percussiveFFT[bin * 2]     = pMag * std::cos (phase);
            percussiveFFT[bin * 2 + 1] = pMag * std::sin (phase);
        }

        fft.performRealOnlyInverseTransform (harmonicFFT.data());
        fft.performRealOnlyInverseTransform (percussiveFFT.data());

        int offset = frame * kHopSize;
        for (int i = 0; i < kFFTSize && (offset + i) < numSamples; ++i)
        {
            harmonicOut[offset + i] += harmonicFFT[i] * (2.0f / 3.0f); // overlap-add normalization
            percussiveOut[offset + i] += percussiveFFT[i] * (2.0f / 3.0f);
        }
    }

    // Noise = original - harmonic - percussive (residual)
    noise.setSize (1, numSamples);
    float* noiseOut = noise.getWritePointer (0);
    for (int i = 0; i < numSamples; ++i)
        noiseOut[i] = input[i] - harmonicOut[i] - percussiveOut[i];
}

void SourceAnalyzer::trackPitch (const juce::AudioBuffer<float>& mono, double sampleRate,
                                  std::vector<float>& pitchContour)
{
    const int numSamples = mono.getNumSamples();
    const int frameSize = kFFTSize;
    const int numFrames = (numSamples - frameSize) / kHopSize + 1;
    const float* input = mono.getReadPointer (0);

    pitchContour.resize (numFrames, 0.0f);

    // Simple autocorrelation-based pitch detection
    int minLag = static_cast<int> (sampleRate / 1000.0);  // ~1000 Hz max
    int maxLag = static_cast<int> (sampleRate / 50.0);    // ~50 Hz min

    for (int frame = 0; frame < numFrames; ++frame)
    {
        int offset = frame * kHopSize;
        if (offset + frameSize > numSamples)
            break;

        // Compute autocorrelation
        float bestCorr = 0.0f;
        int bestLag = 0;

        // RMS for silence detection
        float rms = 0.0f;
        for (int i = 0; i < frameSize; ++i)
            rms += input[offset + i] * input[offset + i];
        rms = std::sqrt (rms / frameSize);

        if (rms < 0.01f) // silence threshold
        {
            pitchContour[frame] = 0.0f;
            continue;
        }

        for (int lag = minLag; lag < maxLag && (offset + lag + frameSize / 2) < numSamples; ++lag)
        {
            float corr = 0.0f;
            float norm1 = 0.0f, norm2 = 0.0f;
            int len = frameSize / 2;

            for (int i = 0; i < len; ++i)
            {
                float s1 = input[offset + i];
                float s2 = input[offset + i + lag];
                corr += s1 * s2;
                norm1 += s1 * s1;
                norm2 += s2 * s2;
            }

            float normFactor = std::sqrt (norm1 * norm2);
            if (normFactor > 0.0f)
                corr /= normFactor;

            if (corr > bestCorr)
            {
                bestCorr = corr;
                bestLag = lag;
            }
        }

        if (bestCorr > 0.5f && bestLag > 0)
            pitchContour[frame] = static_cast<float> (sampleRate) / static_cast<float> (bestLag);
        else
            pitchContour[frame] = 0.0f;
    }
}

void SourceAnalyzer::computeSpectralCentroid (const juce::AudioBuffer<float>& mono, double sampleRate,
                                               std::vector<float>& centroid)
{
    const int numSamples = mono.getNumSamples();
    const int numFrames = (numSamples - kFFTSize) / kHopSize + 1;
    const int numBins = kFFTSize / 2 + 1;
    const float* input = mono.getReadPointer (0);

    centroid.resize (numFrames, 0.0f);

    std::vector<float> fftBuffer (kFFTSize * 2, 0.0f);
    float freqPerBin = static_cast<float> (sampleRate) / kFFTSize;

    for (int frame = 0; frame < numFrames; ++frame)
    {
        int offset = frame * kHopSize;

        std::fill (fftBuffer.begin(), fftBuffer.end(), 0.0f);
        for (int i = 0; i < kFFTSize && (offset + i) < numSamples; ++i)
            fftBuffer[i] = input[offset + i];

        window.multiplyWithWindowingTable (fftBuffer.data(), kFFTSize);
        fft.performRealOnlyForwardTransform (fftBuffer.data());

        float weightedSum = 0.0f;
        float magSum = 0.0f;

        for (int bin = 1; bin < numBins; ++bin)
        {
            float re = fftBuffer[bin * 2];
            float im = fftBuffer[bin * 2 + 1];
            float mag = std::sqrt (re * re + im * im);
            float freq = static_cast<float> (bin) * freqPerBin;

            weightedSum += freq * mag;
            magSum += mag;
        }

        centroid[frame] = (magSum > 0.0f) ? (weightedSum / magSum) : 0.0f;
    }
}

void SourceAnalyzer::detectOnsets (const juce::AudioBuffer<float>& mono, double /*sampleRate*/,
                                    std::vector<float>& onsetStrength)
{
    const int numSamples = mono.getNumSamples();
    const int numFrames = (numSamples - kFFTSize) / kHopSize + 1;
    const int numBins = kFFTSize / 2 + 1;
    const float* input = mono.getReadPointer (0);

    onsetStrength.resize (numFrames, 0.0f);

    std::vector<float> fftBuffer (kFFTSize * 2, 0.0f);
    std::vector<float> prevMag (numBins, 0.0f);

    for (int frame = 0; frame < numFrames; ++frame)
    {
        int offset = frame * kHopSize;

        std::fill (fftBuffer.begin(), fftBuffer.end(), 0.0f);
        for (int i = 0; i < kFFTSize && (offset + i) < numSamples; ++i)
            fftBuffer[i] = input[offset + i];

        window.multiplyWithWindowingTable (fftBuffer.data(), kFFTSize);
        fft.performRealOnlyForwardTransform (fftBuffer.data());

        // Spectral flux (half-wave rectified)
        float flux = 0.0f;
        for (int bin = 0; bin < numBins; ++bin)
        {
            float re = fftBuffer[bin * 2];
            float im = fftBuffer[bin * 2 + 1];
            float mag = std::sqrt (re * re + im * im);
            float diff = mag - prevMag[bin];
            if (diff > 0.0f)
                flux += diff;
            prevMag[bin] = mag;
        }

        onsetStrength[frame] = flux;
    }
}

float SourceAnalyzer::estimateBPM (const std::vector<float>& onsetStrength, double sampleRate)
{
    if (onsetStrength.size() < 2)
        return 0.0f;

    // Autocorrelation of onset strength to find periodicity
    float hopDuration = static_cast<float> (kHopSize) / static_cast<float> (sampleRate);

    // BPM range: 60-200 BPM → period 0.3-1.0 seconds → lag in frames
    int minLag = static_cast<int> (0.3f / hopDuration);
    int maxLag = static_cast<int> (1.0f / hopDuration);
    maxLag = std::min (maxLag, static_cast<int> (onsetStrength.size()) / 2);

    float bestCorr = 0.0f;
    int bestLag = minLag;

    for (int lag = minLag; lag < maxLag; ++lag)
    {
        float corr = 0.0f;
        int count = static_cast<int> (onsetStrength.size()) - lag;
        for (int i = 0; i < count; ++i)
            corr += onsetStrength[i] * onsetStrength[i + lag];

        if (corr > bestCorr)
        {
            bestCorr = corr;
            bestLag = lag;
        }
    }

    float periodSeconds = static_cast<float> (bestLag) * hopDuration;
    return (periodSeconds > 0.0f) ? (60.0f / periodSeconds) : 0.0f;
}

float SourceAnalyzer::medianFilter (const std::vector<float>& data, int pos, int filterSize)
{
    int half = filterSize / 2;
    int n = static_cast<int> (data.size());

    std::vector<float> window;
    window.reserve (filterSize);

    for (int i = pos - half; i <= pos + half; ++i)
    {
        int idx = std::clamp (i, 0, n - 1);
        window.push_back (data[idx]);
    }

    std::sort (window.begin(), window.end());
    return window[window.size() / 2];
}

} // namespace scenememo
