#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <atomic>
#include <functional>

namespace scenememo {

class MomentRecorder
{
public:
    MomentRecorder();
    ~MomentRecorder();

    // Max recording length (2 minutes)
    static constexpr float kMaxRecordLengthSeconds = 120.0f;

    enum class State { Idle, Recording };

    void prepareToPlay (float sampleRate, int blockSize);

    // Start/stop recording. Feed audio via processBlock.
    void startRecording();
    void stopRecording();
    State getState() const { return state.load(); }

    // Feed audio input samples to the recorder (call from processBlock or audio callback).
    // Mono input.
    void processAudioInput (const float* inputSamples, int numSamples);

    // Get the recorded audio buffer (valid after stopRecording)
    const juce::AudioBuffer<float>& getRecordedBuffer() const { return recordBuffer; }
    int getRecordedLength() const { return recordPosition.load(); }
    float getRecordedLengthSeconds() const;

    // Input gain trim (-12dB to +12dB)
    void setInputGain (float gainDb);
    float getInputGainDb() const { return inputGainDb; }

    // Get current input level for metering (0-1)
    float getCurrentInputLevel() const { return inputLevel.load(); }

    // Clear the recorded buffer
    void clearRecording();

private:
    std::atomic<State> state { State::Idle };
    juce::AudioBuffer<float> recordBuffer;
    std::atomic<int> recordPosition { 0 };
    float sampleRate = 44100.0f;
    int maxRecordSamples = 0;

    float inputGainDb = 0.0f;
    float inputGainLinear = 1.0f;
    std::atomic<float> inputLevel { 0.0f };
};

} // namespace scenememo
