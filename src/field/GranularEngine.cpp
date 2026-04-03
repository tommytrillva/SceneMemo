#include "field/GranularEngine.h"
#include <cmath>
#include <algorithm>

namespace scenememo {

// Scale definitions (semitone offsets from root)
static constexpr int kScales[][12] = {
    { 0, 2, 4, 5, 7, 9, 11, -1 },  // Major
    { 0, 2, 3, 5, 7, 8, 10, -1 },  // Minor
    { 0, 2, 3, 5, 7, 9, 10, -1 },  // Dorian
    { 0, 2, 4, 5, 7, 9, 10, -1 },  // Mixolydian
    { 0, 2, 4, 7, 9, -1 },         // Pentatonic Major
    { 0, 3, 5, 7, 10, -1 },        // Pentatonic Minor
    { 0, 2, 3, 5, 7, 8, 11, -1 },  // Harmonic Minor
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 }, // Chromatic
};
static constexpr int kNumScales = 8;

GranularEngine::GranularEngine()
{
    for (auto& g : grains)
        g.active = false;
}

void GranularEngine::prepareToPlay (float sr)
{
    sampleRate = sr;
    reset();
}

void GranularEngine::setSourceBuffer (const juce::AudioBuffer<float>* source, double srcSampleRate)
{
    sourceBuffer = source;
    sourceSampleRate = srcSampleRate;
    reset();
}

void GranularEngine::reset()
{
    for (auto& g : grains)
        g.active = false;
    grainTimer = 0.0f;
    scanPosition = 0.0f;
}

float GranularEngine::nextRng()
{
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    return static_cast<float> (rngState) / static_cast<float> (0xFFFFFFFFu);
}

float GranularEngine::readSourceSample (float position) const
{
    if (sourceBuffer == nullptr || sourceBuffer->getNumSamples() == 0)
        return 0.0f;

    const float* src = sourceBuffer->getReadPointer (0);
    int numSamples = sourceBuffer->getNumSamples();

    // Wrap position
    while (position < 0.0f)
        position += static_cast<float> (numSamples);
    while (position >= static_cast<float> (numSamples))
        position -= static_cast<float> (numSamples);

    int idx = static_cast<int> (position);
    float frac = position - static_cast<float> (idx);
    int next = (idx + 1) % numSamples;

    return src[idx] * (1.0f - frac) + src[next] * frac;
}

float GranularEngine::grainWindowValue (GrainWindow window, float phase)
{
    // phase: 0..1 through the grain
    switch (window)
    {
        case GrainWindow::Hanning:
            return 0.5f * (1.0f - std::cos (phase * 6.28318530718f));

        case GrainWindow::Hamming:
            return 0.54f - 0.46f * std::cos (phase * 6.28318530718f);

        case GrainWindow::Gaussian:
        {
            float x = (phase - 0.5f) * 4.0f; // ±2 sigma
            return std::exp (-0.5f * x * x);
        }

        case GrainWindow::Triangular:
            return (phase < 0.5f) ? (phase * 2.0f) : (2.0f - phase * 2.0f);

        case GrainWindow::Rectangular:
            return 1.0f;
    }
    return 1.0f;
}

void GranularEngine::spawnGrain (const GranularParams& params, int midiNoteForPitch)
{
    if (! hasSource())
        return;

    // Find inactive grain slot
    Grain* slot = nullptr;
    for (auto& g : grains)
    {
        if (! g.active)
        {
            slot = &g;
            break;
        }
    }
    if (slot == nullptr)
        return; // all slots busy

    int srcLen = sourceBuffer->getNumSamples();

    // Grain length in samples
    float grainMs = std::clamp (params.grainSize, 1.0f, 500.0f);
    slot->grainLength = static_cast<int> (grainMs * 0.001f * sampleRate);
    slot->grainLength = std::max (1, slot->grainLength);

    // Source position
    float pos = params.grainPosition;

    // Apply mode-specific position logic
    switch (params.mode)
    {
        case GranularMode::Scrub:
            break; // use grainPosition directly

        case GranularMode::Scan:
            pos = scanPosition;
            break;

        case GranularMode::Freeze:
            break; // locked to grainPosition

        case GranularMode::Scatter:
        {
            float range = 0.2f; // scatter within ±20% of position
            pos += (nextRng() * 2.0f - 1.0f) * range;
            pos = std::clamp (pos, 0.0f, 1.0f);
            break;
        }

        case GranularMode::Sequence:
            break; // position set externally
    }

    // Apply position jitter
    if (params.positionJitter > 0.0f)
    {
        float jitter = (nextRng() * 2.0f - 1.0f) * params.positionJitter * 0.1f;
        pos = std::clamp (pos + jitter, 0.0f, 1.0f);
    }

    slot->sourcePos = static_cast<int> (pos * static_cast<float> (srcLen - 1));
    slot->readPos = static_cast<float> (slot->sourcePos);
    slot->currentSample = 0;

    // Pitch ratio
    float pitchSemitones = params.pitchShift;
    if (midiNoteForPitch >= 0)
    {
        // MIDI-driven: shift from middle C (60) to played note
        pitchSemitones = static_cast<float> (midiNoteForPitch - 60);
    }

    if (params.pitchJitter > 0.0f)
        pitchSemitones += (nextRng() * 2.0f - 1.0f) * params.pitchJitter * 2.0f; // ±200 cents max

    slot->pitchRatio = std::pow (2.0f, pitchSemitones / 12.0f);

    // Sample rate compensation
    if (sourceSampleRate > 0 && std::abs (sourceSampleRate - sampleRate) > 1.0)
        slot->pitchRatio *= static_cast<float> (sourceSampleRate / sampleRate);

    // Level jitter
    slot->amplitude = 1.0f;
    if (params.levelJitter > 0.0f)
        slot->amplitude = 1.0f - nextRng() * params.levelJitter;

    // Pan
    float pan = 0.5f; // center
    if (params.panJitter > 0.0f)
        pan = 0.5f + (nextRng() - 0.5f) * params.panJitter;

    slot->panL = std::cos (pan * 1.5707963f);
    slot->panR = std::sin (pan * 1.5707963f);

    slot->window = params.window;
    slot->active = true;
}

void GranularEngine::triggerGrain (const GranularParams& params)
{
    spawnGrain (params, -1);
}

void GranularEngine::renderBlock (float* outputL, float* outputR, int numSamples,
                                   const GranularParams& params, int midiNoteForPitch)
{
    if (! hasSource())
        return;

    float density = std::clamp (params.grainDensity, 1.0f, 100.0f);
    float grainInterval = sampleRate / density; // samples between grain spawns

    for (int i = 0; i < numSamples; ++i)
    {
        // Spawn new grains based on density
        if (params.mode != GranularMode::Sequence)
        {
            grainTimer -= 1.0f;
            if (grainTimer <= 0.0f)
            {
                spawnGrain (params, midiNoteForPitch);
                grainTimer = grainInterval;
            }
        }

        // Update scan position
        if (params.mode == GranularMode::Scan)
        {
            float scanRate = params.scanSpeed / sampleRate; // normalized per sample
            scanPosition += scanRate * 0.01f;
            if (scanPosition >= 1.0f)
                scanPosition -= 1.0f;
        }

        // Process all active grains
        float sampleL = 0.0f;
        float sampleR = 0.0f;

        for (auto& grain : grains)
        {
            if (! grain.active)
                continue;

            // Window phase (0..1)
            float phase = static_cast<float> (grain.currentSample) / static_cast<float> (grain.grainLength);

            // Window amplitude
            float winAmp = grainWindowValue (grain.window, phase);

            // Read source at current position
            float srcSample = readSourceSample (grain.readPos);

            float grainOut = srcSample * winAmp * grain.amplitude;
            sampleL += grainOut * grain.panL;
            sampleR += grainOut * grain.panR;

            // Advance read position
            grain.readPos += grain.pitchRatio;
            grain.currentSample++;

            // Deactivate when grain is done
            if (grain.currentSample >= grain.grainLength)
                grain.active = false;
        }

        outputL[i] += sampleL;
        outputR[i] += sampleR;
    }
}

float GranularEngine::quantizePitch (float freqHz, int scaleRoot, int scaleType) const
{
    if (freqHz <= 0.0f)
        return freqHz;

    scaleType = std::clamp (scaleType, 0, kNumScales - 1);

    // Convert freq to MIDI note
    float midiNote = 69.0f + 12.0f * std::log2 (freqHz / 440.0f);
    int noteNum = static_cast<int> (std::round (midiNote));

    // Find nearest note in scale
    int pc = ((noteNum % 12) - scaleRoot + 12) % 12; // pitch class relative to root

    int bestDist = 12;
    int bestPC = 0;
    for (int i = 0; i < 12; ++i)
    {
        if (kScales[scaleType][i] < 0)
            break;
        int dist = std::abs (pc - kScales[scaleType][i]);
        dist = std::min (dist, 12 - dist);
        if (dist < bestDist)
        {
            bestDist = dist;
            bestPC = kScales[scaleType][i];
        }
    }

    int quantizedNote = (noteNum / 12) * 12 + scaleRoot + bestPC;
    return 440.0f * std::pow (2.0f, (static_cast<float> (quantizedNote) - 69.0f) / 12.0f);
}

} // namespace scenememo
