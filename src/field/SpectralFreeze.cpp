#include "field/SpectralFreeze.h"
#include <cmath>
#include <algorithm>

namespace scenememo {

SpectralFreeze::SpectralFreeze()
{
    frozenMagnitudes.resize (kMaxBins, 0.0f);
    frozenPhases.resize (kMaxBins, 0.0f);
    partialPhases.resize (kMaxBins, 0.0f);
}

void SpectralFreeze::prepareToPlay (float sr)
{
    sampleRate = sr;
    reset();
}

void SpectralFreeze::reset()
{
    frozen = false;
    std::fill (frozenMagnitudes.begin(), frozenMagnitudes.end(), 0.0f);
    std::fill (frozenPhases.begin(), frozenPhases.end(), 0.0f);
    std::fill (partialPhases.begin(), partialPhases.end(), 0.0f);
}

void SpectralFreeze::freeze (const juce::AudioBuffer<float>& source, int position, int fftSizeChoice)
{
    // FFT size choices: 0=2048, 1=4096, 2=8192, 3=16384
    static constexpr int fftSizes[] = { 2048, 4096, 8192, 16384 };
    int fftOrder = 11 + std::clamp (fftSizeChoice, 0, 3);
    currentFFTSize = fftSizes[std::clamp (fftSizeChoice, 0, 3)];

    if (source.getNumSamples() == 0)
        return;

    const int numBins = currentFFTSize / 2 + 1;
    const float* input = source.getReadPointer (0);
    const int srcLen = source.getNumSamples();

    // Create windowed frame
    std::vector<float> fftBuffer (currentFFTSize * 2, 0.0f);
    juce::dsp::WindowingFunction<float> window (currentFFTSize,
        juce::dsp::WindowingFunction<float>::hann);

    for (int i = 0; i < currentFFTSize; ++i)
    {
        int idx = position + i - currentFFTSize / 2;
        if (idx >= 0 && idx < srcLen)
            fftBuffer[i] = input[idx];
    }

    window.multiplyWithWindowingTable (fftBuffer.data(), currentFFTSize);

    // Forward FFT
    juce::dsp::FFT fft (fftOrder);
    fft.performRealOnlyForwardTransform (fftBuffer.data());

    // Store magnitude and phase
    frozenMagnitudes.resize (numBins);
    frozenPhases.resize (numBins);

    float maxMag = 0.0f;
    for (int bin = 0; bin < numBins; ++bin)
    {
        float re = fftBuffer[bin * 2];
        float im = fftBuffer[bin * 2 + 1];
        frozenMagnitudes[bin] = std::sqrt (re * re + im * im);
        frozenPhases[bin] = std::atan2 (im, re);
        maxMag = std::max (maxMag, frozenMagnitudes[bin]);
    }

    // Normalize magnitudes
    if (maxMag > 0.0f)
    {
        float scale = 1.0f / maxMag;
        for (auto& m : frozenMagnitudes)
            m *= scale;
    }

    // Reset partial phases for clean resynthesis start
    partialPhases.resize (numBins);
    std::fill (partialPhases.begin(), partialPhases.end(), 0.0f);

    frozen = true;
}

void SpectralFreeze::unfreeze()
{
    frozen = false;
}

void SpectralFreeze::applySpectralManipulation (std::vector<float>& mags, const SpectralParams& params) const
{
    int numBins = static_cast<int> (mags.size());

    // Spectral shift: shift all bins up/down
    if (std::abs (params.spectralShift) > 0.01f)
    {
        float shiftRatio = std::pow (2.0f, params.spectralShift / 12.0f);
        std::vector<float> shifted (numBins, 0.0f);

        for (int bin = 1; bin < numBins; ++bin)
        {
            float srcBin = static_cast<float> (bin) / shiftRatio;
            int srcIdx = static_cast<int> (srcBin);
            float frac = srcBin - static_cast<float> (srcIdx);

            if (srcIdx >= 1 && srcIdx + 1 < numBins)
                shifted[bin] = mags[srcIdx] * (1.0f - frac) + mags[srcIdx + 1] * frac;
            else if (srcIdx >= 1 && srcIdx < numBins)
                shifted[bin] = mags[srcIdx];
        }
        mags = shifted;
    }

    // Spectral tilt: brighten or darken
    if (std::abs (params.spectralTilt) > 0.01f)
    {
        for (int bin = 1; bin < numBins; ++bin)
        {
            float normalized = static_cast<float> (bin) / static_cast<float> (numBins);
            float tiltGain = std::pow (normalized, -params.spectralTilt);
            tiltGain = std::clamp (tiltGain, 0.0f, 4.0f);
            mags[bin] *= tiltGain;
        }
    }

    // Spectral blur: average adjacent bins
    if (params.spectralBlur > 0.01f)
    {
        int blurWidth = static_cast<int> (params.spectralBlur * 20.0f) + 1;
        std::vector<float> blurred (numBins, 0.0f);

        for (int bin = 0; bin < numBins; ++bin)
        {
            float sum = 0.0f;
            int count = 0;
            for (int j = -blurWidth; j <= blurWidth; ++j)
            {
                int idx = bin + j;
                if (idx >= 0 && idx < numBins)
                {
                    sum += mags[idx];
                    count++;
                }
            }
            blurred[bin] = sum / static_cast<float> (count);
        }
        mags = blurred;
    }
}

void SpectralFreeze::renderBlock (float* outputL, float* outputR, int numSamples,
                                   const SpectralParams& params, float pitchShiftSemitones)
{
    if (! frozen)
        return;

    int numBins = currentFFTSize / 2 + 1;
    float freqPerBin = sampleRate / static_cast<float> (currentFFTSize);

    // Apply spectral manipulation
    std::vector<float> mags = frozenMagnitudes;
    mags.resize (numBins);
    applySpectralManipulation (mags, params);

    // Pitch shift ratio
    float pitchRatio = std::pow (2.0f, pitchShiftSemitones / 12.0f);

    // Additive resynthesis: sum of sine partials
    // Limit to top N partials for CPU efficiency
    int maxPartials = std::min (numBins, 256);

    for (int i = 0; i < numSamples; ++i)
    {
        float sampleOut = 0.0f;

        for (int bin = 1; bin < maxPartials; ++bin)
        {
            if (mags[bin] < 0.001f)
                continue;

            float freq = static_cast<float> (bin) * freqPerBin * pitchRatio;
            if (freq >= sampleRate * 0.5f)
                break; // above Nyquist

            float phaseInc = freq / sampleRate;
            partialPhases[bin] += phaseInc;
            if (partialPhases[bin] >= 1.0f)
                partialPhases[bin] -= 1.0f;

            sampleOut += mags[bin] * std::sin (partialPhases[bin] * 6.28318530718f);
        }

        // Scale output to reasonable level
        sampleOut *= 0.3f;

        outputL[i] += sampleOut;
        outputR[i] += sampleOut;
    }
}

} // namespace scenememo
