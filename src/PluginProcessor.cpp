#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace scenememo {

SceneMemoProcessor::SceneMemoProcessor()
    : AudioProcessor (BusesProperties()
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "SceneMemoState", createParameterLayout())
{
    // Cache raw parameter pointers
    bypassParam      = apvts.getRawParameterValue (param::kBypass);
    masterVolParam   = apvts.getRawParameterValue (param::kMasterVol);
    dryWetParam      = apvts.getRawParameterValue (param::kDryWet);
    oscLevelParam    = apvts.getRawParameterValue (param::kOscLevel);
    oscTuneParam     = apvts.getRawParameterValue (param::kOscTune);
    oscFineParam     = apvts.getRawParameterValue (param::kOscFine);
    oscWaveformParam = apvts.getRawParameterValue (param::kOscWaveform);
    envAttackParam   = apvts.getRawParameterValue (param::kEnvAttack);
    envHoldParam     = apvts.getRawParameterValue (param::kEnvHold);
    envDecayParam    = apvts.getRawParameterValue (param::kEnvDecay);
    envSustainParam  = apvts.getRawParameterValue (param::kEnvSustain);
    envReleaseParam  = apvts.getRawParameterValue (param::kEnvRelease);
}

SceneMemoProcessor::~SceneMemoProcessor() = default;

void SceneMemoProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    float sr = static_cast<float> (sampleRate);
    voiceAllocator.prepareToPlay (sr, samplesPerBlock);
    masterVolSmoothed.init (masterVolParam, kParamSmoothingSeconds, sr);
}

void SceneMemoProcessor::releaseResources()
{
    voiceAllocator.reset();
}

void SceneMemoProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    // Clear output
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    // Check bypass
    if (bypassParam->load() >= 0.5f)
        return;

    // Read parameters (once per block)
    float oscLevel = oscLevelParam->load();
    float tune     = oscTuneParam->load();
    float fine     = oscFineParam->load();
    int waveform   = static_cast<int> (oscWaveformParam->load());

    AHDSREnvelope::Parameters envParams;
    envParams.attack  = envAttackParam->load();
    envParams.hold    = envHoldParam->load();
    envParams.decay   = envDecayParam->load();
    envParams.sustain = envSustainParam->load();
    envParams.release = envReleaseParam->load();

    // Process voices
    voiceAllocator.processBlock (buffer, midi, oscLevel, tune, fine, waveform, envParams);

    // Apply smoothed master volume
    masterVolSmoothed.updateTarget();
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    for (int i = 0; i < numSamples; ++i)
    {
        float vol = masterVolSmoothed.getNextValue();
        for (int ch = 0; ch < numChannels; ++ch)
            buffer.getWritePointer (ch)[i] *= vol;
    }
}

juce::AudioProcessorEditor* SceneMemoProcessor::createEditor()
{
    return new SceneMemoEditor (*this);
}

void SceneMemoProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void SceneMemoProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorValueTreeState::ParameterLayout SceneMemoProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Bypass
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { param::kBypass, 1 }, "Bypass", false));

    // Master Volume (0–1, default 0.8)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { param::kMasterVol, 1 }, "Master Volume",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.0f, 0.5f), // skew for more resolution at lower volumes
        0.8f));

    // Dry/Wet (0–1, default 1.0 = fully wet for a synth)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { param::kDryWet, 1 }, "Dry/Wet",
        juce::NormalisableRange<float> (0.0f, 1.0f),
        1.0f));

    // Osc Level (0–1, default 0.8)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { param::kOscLevel, 1 }, "Osc Level",
        juce::NormalisableRange<float> (0.0f, 1.0f),
        0.8f));

    // Osc Tune (-24 to +24 semitones)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { param::kOscTune, 1 }, "Osc Tune",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 1.0f),
        0.0f));

    // Osc Fine Tune (-100 to +100 cents)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { param::kOscFine, 1 }, "Osc Fine",
        juce::NormalisableRange<float> (-100.0f, 100.0f, 0.1f),
        0.0f));

    // Waveform selector
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { param::kOscWaveform, 1 }, "Waveform",
        juce::StringArray { "Sine", "Saw", "Square", "Triangle" },
        0));

    // Envelope: Attack
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { param::kEnvAttack, 1 }, "Attack",
        juce::NormalisableRange<float> (kMinEnvTime, kMaxAttack, 0.0f, 0.4f),
        kDefaultAttack));

    // Envelope: Hold
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { param::kEnvHold, 1 }, "Hold",
        juce::NormalisableRange<float> (0.0f, kMaxHold, 0.0f, 0.5f),
        kDefaultHold));

    // Envelope: Decay
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { param::kEnvDecay, 1 }, "Decay",
        juce::NormalisableRange<float> (kMinEnvTime, kMaxDecay, 0.0f, 0.4f),
        kDefaultDecay));

    // Envelope: Sustain (level 0–1)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { param::kEnvSustain, 1 }, "Sustain",
        juce::NormalisableRange<float> (0.0f, 1.0f),
        kDefaultSustain));

    // Envelope: Release
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { param::kEnvRelease, 1 }, "Release",
        juce::NormalisableRange<float> (kMinEnvTime, kMaxRelease, 0.0f, 0.4f),
        kDefaultRelease));

    return { params.begin(), params.end() };
}

} // namespace scenememo

// JUCE requires this factory function at global scope
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new scenememo::SceneMemoProcessor();
}
