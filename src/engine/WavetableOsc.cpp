#include "engine/WavetableOsc.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <functional>

namespace scenememo {

//==============================================================================
// Wavetable
//==============================================================================

Wavetable::Wavetable (const std::vector<float>& rawSamples)
{
    buildMipLevels (rawSamples);
}

const Wavetable::MipLevel& Wavetable::getMipLevel (int level) const
{
    level = std::clamp (level, 0, static_cast<int> (mipLevels.size()) - 1);
    return mipLevels[static_cast<size_t> (level)];
}

void Wavetable::buildMipLevels (const std::vector<float>& baseTable)
{
    const int baseSize = static_cast<int> (baseTable.size());
    assert (baseSize > 0 && (baseSize & (baseSize - 1)) == 0); // must be power of 2

    // Level 0 = full resolution copy
    mipLevels.clear();
    mipLevels.push_back ({ baseTable, baseSize });

    // Build successive mip levels by halving harmonics via spectral method.
    // We use a simple time-domain approach: average adjacent pairs and
    // apply a basic low-pass (2-point average is a crude half-band filter).
    // For production quality, FFT-based would be better, but this is
    // sufficient for Phase 1 since we generate band-limited waveforms
    // via additive synthesis already.
    auto currentTable = baseTable;
    int currentSize = baseSize;

    while (currentSize > 4 && static_cast<int> (mipLevels.size()) < kMaxMipLevels)
    {
        int newSize = currentSize / 2;
        std::vector<float> newTable (static_cast<size_t> (newSize));

        for (int i = 0; i < newSize; ++i)
            newTable[static_cast<size_t> (i)] = 0.5f * (currentTable[static_cast<size_t> (i * 2)]
                                                       + currentTable[static_cast<size_t> (i * 2 + 1)]);

        mipLevels.push_back ({ newTable, newSize });
        currentTable = std::move (newTable);
        currentSize = newSize;
    }
}

//==============================================================================
// WavetableOsc
//==============================================================================

void WavetableOsc::setWavetable (std::shared_ptr<const Wavetable> wt)
{
    wavetable = std::move (wt);
}

void WavetableOsc::setFrequency (float freqHz, float sr)
{
    currentFrequency = freqHz;
    sampleRate = sr;
    phaseIncrement = freqHz / sr;
}

void WavetableOsc::reset()
{
    phase = 0.0f;
}

float WavetableOsc::processSample()
{
    if (wavetable == nullptr)
        return 0.0f;

    int mipLevel = selectMipLevel();
    const auto& level = wavetable->getMipLevel (mipLevel);
    float sample = interpolateSample (level, phase);

    phase += phaseIncrement;
    if (phase >= 1.0f)
        phase -= 1.0f;
    if (phase < 0.0f)
        phase += 1.0f;

    return sample;
}

int WavetableOsc::selectMipLevel() const
{
    if (wavetable == nullptr || wavetable->getNumMipLevels() == 0)
        return 0;

    // phaseIncrement = freq / sampleRate = the fraction of table traversed per sample.
    // When phaseIncrement > 1/tableSize, we're skipping samples -> need higher mip level.
    // level = floor(log2(phaseIncrement * tableSize))
    const int baseSize = wavetable->getMipLevel (0).size;
    float tablesPerSample = phaseIncrement * static_cast<float> (baseSize);

    if (tablesPerSample <= 1.0f)
        return 0;

    int level = static_cast<int> (std::floor (std::log2 (tablesPerSample)));
    return std::clamp (level, 0, wavetable->getNumMipLevels() - 1);
}

float WavetableOsc::interpolateSample (const Wavetable::MipLevel& level, float normalizedPhase)
{
    float pos = normalizedPhase * static_cast<float> (level.size);
    int index0 = static_cast<int> (pos);
    float frac = pos - static_cast<float> (index0);

    index0 = index0 % level.size;
    int index1 = (index0 + 1) % level.size;

    // Linear interpolation
    return level.samples[static_cast<size_t> (index0)] * (1.0f - frac)
         + level.samples[static_cast<size_t> (index1)] * frac;
}

//==============================================================================
// WavetableFactory — additive synthesis for band-limited waveforms
//==============================================================================

static std::vector<float> generateBandLimitedWave (int size,
    std::function<float(int harmonic, int maxHarmonics)> amplitudeFunc)
{
    std::vector<float> table (static_cast<size_t> (size), 0.0f);
    const int maxHarmonics = size / 2; // Nyquist limit for the base table
    const float twoPi = 2.0f * static_cast<float> (M_PI);

    for (int h = 1; h <= maxHarmonics; ++h)
    {
        float amp = amplitudeFunc (h, maxHarmonics);
        if (std::abs (amp) < 1e-10f)
            continue;

        for (int i = 0; i < size; ++i)
        {
            float phase = twoPi * static_cast<float> (h * i) / static_cast<float> (size);
            table[static_cast<size_t> (i)] += amp * std::sin (phase);
        }
    }

    // Normalize to [-1, 1]
    float maxVal = 0.0f;
    for (auto s : table)
        maxVal = std::max (maxVal, std::abs (s));

    if (maxVal > 0.0f)
    {
        float scale = 1.0f / maxVal;
        for (auto& s : table)
            s *= scale;
    }

    return table;
}

std::shared_ptr<const Wavetable> WavetableFactory::createSine (int size)
{
    auto table = generateBandLimitedWave (size, [] (int h, int) -> float {
        return (h == 1) ? 1.0f : 0.0f;
    });
    return std::make_shared<const Wavetable> (table);
}

std::shared_ptr<const Wavetable> WavetableFactory::createSaw (int size)
{
    auto table = generateBandLimitedWave (size, [] (int h, int) -> float {
        // Sawtooth: sum of sin(n*x)/n with alternating sign for downward saw
        return (1.0f / static_cast<float> (h)) * ((h % 2 == 0) ? -1.0f : 1.0f);
    });
    return std::make_shared<const Wavetable> (table);
}

std::shared_ptr<const Wavetable> WavetableFactory::createSquare (int size)
{
    auto table = generateBandLimitedWave (size, [] (int h, int) -> float {
        // Square: odd harmonics only, amplitude 1/n
        return (h % 2 != 0) ? (1.0f / static_cast<float> (h)) : 0.0f;
    });
    return std::make_shared<const Wavetable> (table);
}

std::shared_ptr<const Wavetable> WavetableFactory::createTriangle (int size)
{
    auto table = generateBandLimitedWave (size, [] (int h, int) -> float {
        // Triangle: odd harmonics, amplitude 1/n^2, alternating sign
        if (h % 2 == 0)
            return 0.0f;
        float sign = ((h / 2) % 2 == 0) ? 1.0f : -1.0f;
        return sign / static_cast<float> (h * h);
    });
    return std::make_shared<const Wavetable> (table);
}

} // namespace scenememo
