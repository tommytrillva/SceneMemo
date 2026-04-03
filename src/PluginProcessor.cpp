#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "engine/ParameterLayout.h"

namespace scenememo {

SceneMemoProcessor::SceneMemoProcessor()
    : AudioProcessor (BusesProperties()
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "SceneMemoState", params::createFullLayout())
{
    paramReader.cachePointers (apvts);
}

SceneMemoProcessor::~SceneMemoProcessor() = default;

void SceneMemoProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    float sr = static_cast<float> (sampleRate);
    voiceAllocator.prepareToPlay (sr, samplesPerBlock);
    masterVolSmoothed.init (apvts.getRawParameterValue (param::kMasterVol),
                            kParamSmoothingSeconds, sr);
}

void SceneMemoProcessor::releaseResources()
{
    voiceAllocator.reset();
}

void SceneMemoProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    // Read all parameters once
    paramReader.readAll (engineParams);

    if (engineParams.bypass)
        return;

    // Read host transport
    float bpm = 120.0f;
    double ppqPosition = 0.0;
    if (auto* playHead = getPlayHead())
    {
        auto pos = playHead->getPosition();
        if (pos.hasValue())
        {
            if (pos->getBpm().hasValue())
                bpm = static_cast<float> (*pos->getBpm());
            if (pos->getPpqPosition().hasValue())
                ppqPosition = *pos->getPpqPosition();
        }
    }

    // Process voices
    voiceAllocator.processBlock (buffer, midi, engineParams, bpm, ppqPosition);

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

} // namespace scenememo

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new scenememo::SceneMemoProcessor();
}
