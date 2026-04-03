#include "field/MomentRecorder.h"
#include <cmath>
#include <algorithm>

namespace scenememo {

MomentRecorder::MomentRecorder() = default;
MomentRecorder::~MomentRecorder() = default;

void MomentRecorder::prepareToPlay (float sr, int /*blockSize*/)
{
    sampleRate = sr;
    maxRecordSamples = static_cast<int> (kMaxRecordLengthSeconds * sr);
    recordBuffer.setSize (1, maxRecordSamples);
    recordBuffer.clear();
    recordPosition.store (0);
}

void MomentRecorder::startRecording()
{
    recordPosition.store (0);
    recordBuffer.clear();
    state.store (State::Recording);
}

void MomentRecorder::stopRecording()
{
    state.store (State::Idle);
}

void MomentRecorder::processAudioInput (const float* inputSamples, int numSamples)
{
    // Update input level for metering
    float peak = 0.0f;
    for (int i = 0; i < numSamples; ++i)
        peak = std::max (peak, std::abs (inputSamples[i]));
    inputLevel.store (peak * inputGainLinear);

    if (state.load() != State::Recording)
        return;

    int pos = recordPosition.load();
    float* buffer = recordBuffer.getWritePointer (0);

    int samplesToWrite = std::min (numSamples, maxRecordSamples - pos);
    if (samplesToWrite <= 0)
    {
        stopRecording();
        return;
    }

    for (int i = 0; i < samplesToWrite; ++i)
        buffer[pos + i] = inputSamples[i] * inputGainLinear;

    recordPosition.store (pos + samplesToWrite);
}

float MomentRecorder::getRecordedLengthSeconds() const
{
    return (sampleRate > 0.0f) ? (static_cast<float> (recordPosition.load()) / sampleRate) : 0.0f;
}

void MomentRecorder::setInputGain (float gainDb)
{
    inputGainDb = std::clamp (gainDb, -12.0f, 12.0f);
    inputGainLinear = std::pow (10.0f, inputGainDb / 20.0f);
}

void MomentRecorder::clearRecording()
{
    state.store (State::Idle);
    recordPosition.store (0);
    recordBuffer.clear();
}

} // namespace scenememo
