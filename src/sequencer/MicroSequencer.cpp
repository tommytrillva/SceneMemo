#include "sequencer/MicroSequencer.h"
#include <cmath>
#include <algorithm>

namespace scenememo {

MicroSequencer::MicroSequencer() = default;

void MicroSequencer::prepareToPlay (float sr)
{
    sampleRate = sr;
    reset();
}

void MicroSequencer::reset()
{
    lastStep = -1;
}

float MicroSequencer::nextRng()
{
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    return static_cast<float> (rngState) / static_cast<float> (0xFFFFFFFFu);
}

void MicroSequencer::setStep (int patternSlot, int stepIndex, const MicroStep& step)
{
    patternSlot = std::clamp (patternSlot, 0, kNumPatterns - 1);
    stepIndex = std::clamp (stepIndex, 0, kMaxSteps - 1);
    patterns[patternSlot][stepIndex] = step;
}

MicroStep MicroSequencer::getStep (int patternSlot, int stepIndex) const
{
    patternSlot = std::clamp (patternSlot, 0, kNumPatterns - 1);
    stepIndex = std::clamp (stepIndex, 0, kMaxSteps - 1);
    return patterns[patternSlot][stepIndex];
}

MicroSequencer::TriggeredEvent MicroSequencer::processBlock (
    int /*numSamples*/, float bpm, double ppqPosition,
    const MicroSequencerParams& params)
{
    TriggeredEvent result;

    if (! params.enabled || bpm <= 0.0 || params.patternLength <= 0)
        return result;

    // Step resolution in beats: 1/4=1.0, 1/8=0.5, 1/16=0.25, 1/32=0.125
    static constexpr float resolutions[] = { 1.0f, 0.5f, 0.25f, 0.125f };
    float stepBeats = resolutions[std::clamp (params.stepResolution, 0, 3)];

    // Calculate current step from PPQ position
    double stepsPerBeat = 1.0 / static_cast<double> (stepBeats);
    int currentStep = static_cast<int> (std::fmod (ppqPosition * stepsPerBeat, params.patternLength));

    if (currentStep == lastStep)
        return result; // same step, no trigger

    lastStep = currentStep;

    const auto& step = patterns[activePattern][currentStep];

    if (step.event == TextureEvent::Nothing)
        return result;

    // Probability check
    if (step.probability < 1.0f && nextRng() > step.probability)
        return result;

    result.event = step.event;
    result.intensity = step.intensity;
    result.triggered = true;
    return result;
}

} // namespace scenememo
