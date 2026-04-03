#include "engine/MultiModeFilter.h"
#include <cmath>
#include <algorithm>

namespace scenememo {

void MultiModeFilter::prepareToPlay (float sr)
{
    sampleRate = sr;
    reset();
}

void MultiModeFilter::reset()
{
    for (auto& s : svfStages)
    {
        s.ic1eq = 0.0f;
        s.ic2eq = 0.0f;
    }
    combBuffer.fill (0.0f);
    combWritePos = 0;
    for (auto& fb : formantBands)
    {
        fb.state.ic1eq = 0.0f;
        fb.state.ic2eq = 0.0f;
    }
}

float MultiModeFilter::softClip (float x)
{
    // Fast tanh approximation
    if (x > 3.0f) return 1.0f;
    if (x < -3.0f) return -1.0f;
    float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

// SVF (Cytomic / Andy Simper) — mode: 0=LP, 1=HP, 2=BP, 3=Notch
float MultiModeFilter::processSVF (SVFState& state, float input, float cutoffHz, float q, int mode)
{
    // Clamp cutoff to Nyquist
    cutoffHz = std::clamp (cutoffHz, 20.0f, sampleRate * 0.49f);

    float g = std::tan (3.14159265f * cutoffHz / sampleRate);
    float k = 1.0f / std::max (q, 0.5f); // Q: 0.5 = no resonance, higher = more resonance

    float a1 = 1.0f / (1.0f + g * (g + k));
    float a2 = g * a1;
    float a3 = g * a2;

    float v3 = input - state.ic2eq;
    float v1 = a1 * state.ic1eq + a2 * v3;
    float v2 = state.ic2eq + a2 * state.ic1eq + a3 * v3;

    state.ic1eq = 2.0f * v1 - state.ic1eq;
    state.ic2eq = 2.0f * v2 - state.ic2eq;

    switch (mode)
    {
        case 0: return v2;                          // lowpass
        case 1: return input - k * v1 - v2;        // highpass
        case 2: return v1;                          // bandpass
        case 3: return input - k * v1;              // notch
        default: return v2;
    }
}

float MultiModeFilter::processComb (float input, float cutoffHz, float feedback)
{
    // Comb filter: delay = 1/frequency
    cutoffHz = std::clamp (cutoffHz, 20.0f, sampleRate * 0.49f);
    float delaySamples = sampleRate / cutoffHz;
    int delayInt = static_cast<int> (delaySamples);
    float delayFrac = delaySamples - static_cast<float> (delayInt);

    // Read from delay buffer with linear interpolation
    int readPos1 = (combWritePos - delayInt + kCombBufferSize) % kCombBufferSize;
    int readPos2 = (readPos1 - 1 + kCombBufferSize) % kCombBufferSize;
    float delayed = combBuffer[readPos1] * (1.0f - delayFrac) + combBuffer[readPos2] * delayFrac;

    float output = input + delayed * feedback;
    output = softClip (output); // prevent runaway

    combBuffer[combWritePos] = output;
    combWritePos = (combWritePos + 1) % kCombBufferSize;

    return output;
}

float MultiModeFilter::processFormant (float input, float morph)
{
    // Vowel formant frequencies (A, E, I, O, U)
    static constexpr float vowelFreqs[5][3] = {
        { 800.0f, 1150.0f, 2900.0f },  // A
        { 350.0f, 2000.0f, 2800.0f },  // E
        { 270.0f, 2140.0f, 2950.0f },  // I
        { 450.0f, 800.0f,  2830.0f },  // O
        { 325.0f, 700.0f,  2700.0f },  // U
    };

    // Morph between vowels (0-1 maps across 5 vowels)
    float vowelPos = morph * 4.0f;
    int vowel1 = std::clamp (static_cast<int> (vowelPos), 0, 3);
    int vowel2 = vowel1 + 1;
    float vowelFrac = vowelPos - static_cast<float> (vowel1);

    float output = 0.0f;
    for (int b = 0; b < 3; ++b)
    {
        float freq = vowelFreqs[vowel1][b] * (1.0f - vowelFrac) + vowelFreqs[vowel2][b] * vowelFrac;
        output += processSVF (formantBands[b].state, input, freq, 5.0f, 2) * 0.33f; // BP mode
    }
    return output;
}

float MultiModeFilter::processSample (float input, FilterType type, float cutoff, float resonance,
                                       float drive, float keyTrack, float keyFreq)
{
    // Apply key tracking
    if (keyTrack > 0.0f && keyFreq > 0.0f)
    {
        float keyOffset = (keyFreq - 261.63f) * keyTrack; // relative to middle C
        cutoff += keyOffset;
    }

    cutoff = std::clamp (cutoff, 20.0f, 20000.0f);

    // Apply drive (pre-filter saturation)
    if (drive > 0.0f)
        input = softClip (input * (1.0f + drive * 4.0f));

    // Map resonance 0-1 to Q (0.5 to 20)
    float q = 0.5f + resonance * 19.5f;

    switch (type)
    {
        case FilterType::LP12:
            return processSVF (svfStages[0], input, cutoff, q, 0);

        case FilterType::LP24:
        {
            float out = processSVF (svfStages[0], input, cutoff, q, 0);
            return processSVF (svfStages[1], out, cutoff, q, 0);
        }

        case FilterType::LP36:
        {
            float out = processSVF (svfStages[0], input, cutoff, q, 0);
            out = processSVF (svfStages[1], out, cutoff, q, 0);
            return processSVF (svfStages[2], out, cutoff, q, 0);
        }

        case FilterType::HP12:
            return processSVF (svfStages[0], input, cutoff, q, 1);

        case FilterType::HP24:
        {
            float out = processSVF (svfStages[0], input, cutoff, q, 1);
            return processSVF (svfStages[1], out, cutoff, q, 1);
        }

        case FilterType::BP:
            return processSVF (svfStages[0], input, cutoff, q, 2);

        case FilterType::Notch:
            return processSVF (svfStages[0], input, cutoff, q, 3);

        case FilterType::Comb:
            return processComb (input, cutoff, resonance);

        case FilterType::Formant:
            return processFormant (input, cutoff / 20000.0f); // map cutoff to morph 0-1

        default:
            return input;
    }
}

} // namespace scenememo
