#include "engine/LFO.h"
#include <cmath>
#include <algorithm>

namespace scenememo {

void LFO::prepareToPlay (float sr)
{
    sampleRate = sr;
}

void LFO::noteOn (float phaseOffset, LFORetrigger retrigger)
{
    if (retrigger == LFORetrigger::NoteOn
        || (retrigger == LFORetrigger::FirstNote && firstSample))
    {
        phase = phaseOffset;
        fadeInLevel = 0.0f;
    }
    firstSample = false;
}

void LFO::reset()
{
    phase = 0.0f;
    currentValue = 0.0f;
    fadeInLevel = 1.0f;
    fadeInRate = 0.0f;
    firstSample = true;
    prevRandom = 0.0f;
    nextRandom = nextRng();
}

float LFO::processSample (LFOShape shape, float rate, bool tempoSync,
                           int syncDivIndex, float fadeInTime, float humanize,
                           float bpm, double /*ppqPosition*/)
{
    // Compute effective rate
    float effectiveRate = rate;
    if (tempoSync && bpm > 0.0f)
    {
        int idx = std::clamp (syncDivIndex, 0, kNumSyncDivisions - 1);
        float bars = kSyncDivisions[idx].bars;
        float beatsPerBar = 4.0f; // assume 4/4
        float secondsPerBar = (beatsPerBar * 60.0f) / bpm;
        float periodSeconds = bars * secondsPerBar;
        effectiveRate = (periodSeconds > 0.0f) ? (1.0f / periodSeconds) : rate;
    }

    // Apply humanize (subtle random deviation to rate)
    if (humanize > 0.0f)
    {
        float deviation = (nextRng() - 0.5f) * 2.0f * humanize * 0.05f;
        effectiveRate *= (1.0f + deviation);
    }

    float phaseInc = effectiveRate / sampleRate;

    // Advance phase
    float prevPhase = phase;
    phase += phaseInc;

    // Handle S&H: generate new random value on phase wrap
    if (phase >= 1.0f)
    {
        phase -= std::floor (phase);
        if (shape == LFOShape::SampleHold || shape == LFOShape::RandomSmooth)
        {
            prevRandom = nextRandom;
            nextRandom = nextRng() * 2.0f - 1.0f; // -1 to +1
        }
    }

    // Compute raw LFO value
    currentValue = computeShape (shape, phase);

    // Apply fade-in
    if (fadeInTime > 0.0f && fadeInLevel < 1.0f)
    {
        fadeInRate = 1.0f / (fadeInTime * sampleRate);
        fadeInLevel = std::min (1.0f, fadeInLevel + fadeInRate);
        currentValue *= fadeInLevel;
    }

    return currentValue;
}

float LFO::computeShape (LFOShape shape, float ph) const
{
    switch (shape)
    {
        case LFOShape::Sine:
            return std::sin (ph * 2.0f * 3.14159265f);

        case LFOShape::Triangle:
        {
            if (ph < 0.25f)      return ph * 4.0f;
            else if (ph < 0.75f) return 2.0f - ph * 4.0f;
            else                 return ph * 4.0f - 4.0f;
        }

        case LFOShape::SawUp:
            return 2.0f * ph - 1.0f;

        case LFOShape::SawDown:
            return 1.0f - 2.0f * ph;

        case LFOShape::Square:
            return ph < 0.5f ? 1.0f : -1.0f;

        case LFOShape::SampleHold:
            return nextRandom;

        case LFOShape::RandomSmooth:
        {
            // Linear interpolation between random values
            return prevRandom + (nextRandom - prevRandom) * ph;
        }

        case LFOShape::Custom32:
            return 0.0f; // placeholder for user-drawable LFO

        default:
            return 0.0f;
    }
}

float LFO::nextRng()
{
    // xorshift32
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    return static_cast<float> (rngState) / static_cast<float> (0xFFFFFFFFu);
}

} // namespace scenememo
