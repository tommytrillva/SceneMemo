#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "engine/VoiceAllocator.h"
#include "util/SmoothedParam.h"
#include "util/Constants.h"

namespace scenememo {

class SceneMemoProcessor : public juce::AudioProcessor
{
public:
    SceneMemoProcessor();
    ~SceneMemoProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using juce::AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

private:
    juce::AudioProcessorValueTreeState apvts;
    VoiceAllocator voiceAllocator;

    // Raw parameter pointers (into APVTS atomics)
    std::atomic<float>* bypassParam    = nullptr;
    std::atomic<float>* masterVolParam = nullptr;
    std::atomic<float>* dryWetParam    = nullptr;
    std::atomic<float>* oscLevelParam  = nullptr;
    std::atomic<float>* oscTuneParam   = nullptr;
    std::atomic<float>* oscFineParam   = nullptr;
    std::atomic<float>* oscWaveformParam = nullptr;
    std::atomic<float>* envAttackParam  = nullptr;
    std::atomic<float>* envHoldParam    = nullptr;
    std::atomic<float>* envDecayParam   = nullptr;
    std::atomic<float>* envSustainParam = nullptr;
    std::atomic<float>* envReleaseParam = nullptr;

    // Smoothed master volume
    SmoothedParam masterVolSmoothed;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneMemoProcessor)
};

} // namespace scenememo
