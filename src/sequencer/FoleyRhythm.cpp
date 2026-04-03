#include "sequencer/FoleyRhythm.h"
#include <cmath>
#include <algorithm>

namespace scenememo {

FoleyRhythm::FoleyRhythm() = default;

void FoleyRhythm::prepareToPlay (float sr)
{
    sampleRate = sr;
    reset();
}

void FoleyRhythm::reset()
{
    lastStep = -1;
    for (auto& v : playbackVoices)
        v.active = false;
}

float FoleyRhythm::nextRng()
{
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    return static_cast<float> (rngState) / static_cast<float> (0xFFFFFFFFu);
}

void FoleyRhythm::loadSlot (int slotIndex, const juce::AudioBuffer<float>& audio, int numSamples)
{
    if (slotIndex < 0 || slotIndex >= kNumSlots)
        return;

    auto& slot = slots[slotIndex];
    slot.buffer.setSize (1, numSamples);
    slot.buffer.copyFrom (0, 0, audio, 0, 0, numSamples);
    slot.numSamples = numSamples;
    slot.startSample = 0;
    slot.endSample = numSamples;
    slot.loaded = true;
}

void FoleyRhythm::clearSlot (int slotIndex)
{
    if (slotIndex < 0 || slotIndex >= kNumSlots)
        return;

    slots[slotIndex].loaded = false;
    slots[slotIndex].numSamples = 0;
}

void FoleyRhythm::setStep (int patternSlot, int stepIndex, const FoleyStep& step)
{
    patternSlot = std::clamp (patternSlot, 0, kNumPatterns - 1);
    stepIndex = std::clamp (stepIndex, 0, kMaxSteps - 1);
    patterns[patternSlot][stepIndex] = step;
}

FoleyStep FoleyRhythm::getStep (int patternSlot, int stepIndex) const
{
    patternSlot = std::clamp (patternSlot, 0, kNumPatterns - 1);
    stepIndex = std::clamp (stepIndex, 0, kMaxSteps - 1);
    return patterns[patternSlot][stepIndex];
}

void FoleyRhythm::triggerSlot (int slotIndex, float velocity, float pitchOffset)
{
    if (slotIndex < 0 || slotIndex >= kNumSlots)
        return;
    if (! slots[slotIndex].loaded)
        return;

    auto& voice = playbackVoices[slotIndex];
    voice.active = true;
    voice.slotIndex = slotIndex;
    voice.readPos = static_cast<float> (slots[slotIndex].startSample);
    voice.pitchRatio = std::pow (2.0f, (slots[slotIndex].pitch + pitchOffset) / 12.0f);
    voice.velocity = velocity * slots[slotIndex].level;

    float panAngle = (slots[slotIndex].pan + 1.0f) * 0.5f;
    voice.panL = std::cos (panAngle * 1.5707963f);
    voice.panR = std::sin (panAngle * 1.5707963f);
}

void FoleyRhythm::processBlock (float* outputL, float* outputR, int numSamples,
                                  float bpm, double ppqPosition, const FoleyRhythmParams& params)
{
    // Step sequencer logic
    if (params.enabled && bpm > 0.0)
    {
        static constexpr float resolutions[] = { 1.0f, 0.5f, 0.25f, 0.125f };
        float stepBeats = resolutions[std::clamp (params.stepResolution, 0, 3)];
        double stepsPerBeat = 1.0 / static_cast<double> (stepBeats);
        int currentStep = static_cast<int> (std::fmod (ppqPosition * stepsPerBeat, params.patternLength));

        if (currentStep != lastStep)
        {
            lastStep = currentStep;

            const auto& step = patterns[activePattern][currentStep];
            if (step.slotIndex >= 0 && step.slotIndex < kNumSlots)
            {
                // Probability check
                if (step.probability >= 1.0f || nextRng() <= step.probability)
                {
                    float vel = step.velocity;
                    if (params.humanizeVelocity > 0.0f)
                        vel += (nextRng() * 2.0f - 1.0f) * params.humanizeVelocity * 0.3f;
                    vel = std::clamp (vel, 0.0f, 1.0f);

                    triggerSlot (step.slotIndex, vel, step.pitchOffset);
                }
            }
        }
    }

    // Render active playback voices
    for (auto& voice : playbackVoices)
    {
        if (! voice.active || voice.slotIndex < 0)
            continue;

        const auto& slot = slots[voice.slotIndex];
        if (! slot.loaded)
        {
            voice.active = false;
            continue;
        }

        const float* src = slot.buffer.getReadPointer (0);
        int endPos = slot.endSample;

        for (int i = 0; i < numSamples; ++i)
        {
            int readIdx = static_cast<int> (voice.readPos);
            if (readIdx >= endPos)
            {
                voice.active = false;
                break;
            }

            float frac = voice.readPos - static_cast<float> (readIdx);
            int next = std::min (readIdx + 1, endPos - 1);
            float sample = src[readIdx] * (1.0f - frac) + src[next] * frac;

            sample *= voice.velocity;
            outputL[i] += sample * voice.panL;
            outputR[i] += sample * voice.panR;

            voice.readPos += voice.pitchRatio;
        }
    }
}

} // namespace scenememo
