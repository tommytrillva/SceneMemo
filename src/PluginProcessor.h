#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "engine/VoiceAllocator.h"
#include "engine/ParamReader.h"
#include "field/FieldEngine.h"
#include "mixer/BlendMatrix.h"
#include "mixer/SceneMorph.h"
#include "fx/GritSection.h"
#include "fx/MotionFX.h"
#include "fx/SidechainModule.h"
#include "fx/Reverb.h"
#include "fx/Delay.h"
#include "fx/MasterOutput.h"
#include "preset/PresetManager.h"
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
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    FieldEngine& getFieldEngine() { return fieldEngine; }
    PresetManager& getPresetManager() { return presetManager; }
    SceneMorph& getSceneMorph() { return sceneMorph; }

private:
    juce::AudioProcessorValueTreeState apvts;

    // Engines
    VoiceAllocator voiceAllocator;
    FieldEngine fieldEngine;

    // Mixer
    BlendMatrix blendMatrix;
    SceneMorph sceneMorph;

    // FX chain
    GritSection gritSection;
    MotionFX motionFX;
    SidechainModule sidechainModule;
    ReverbEffect reverbEffect;
    DelayEffect delayEffect;
    MasterOutput masterOutput;

    // Presets
    PresetManager presetManager;

    // Parameters
    ParamReader paramReader;
    EngineParams engineParams;
    SmoothedParam masterVolSmoothed;

    float currentSampleRate = 44100.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneMemoProcessor)
};

} // namespace scenememo
