#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "engine/ParameterLayout.h"

namespace scenememo {

SceneMemoProcessor::SceneMemoProcessor()
    : AudioProcessor (BusesProperties()
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "SceneMemoState", params::createFullLayout()),
      presetManager (apvts)
{
    paramReader.cachePointers (apvts);
    presetManager.scanPresets();
}

SceneMemoProcessor::~SceneMemoProcessor() = default;

void SceneMemoProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = static_cast<float> (sampleRate);
    voiceAllocator.prepareToPlay (currentSampleRate, samplesPerBlock);
    fieldEngine.prepareToPlay (currentSampleRate, samplesPerBlock);
    gritSection.prepareToPlay (currentSampleRate);
    motionFX.prepareToPlay (currentSampleRate);
    sidechainModule.prepareToPlay (currentSampleRate);
    reverbEffect.prepareToPlay (currentSampleRate, samplesPerBlock);
    delayEffect.prepareToPlay (currentSampleRate);
    masterOutput.prepareToPlay (currentSampleRate);
    masterVolSmoothed.init (apvts.getRawParameterValue (param::kMasterVol),
                            kParamSmoothingSeconds, currentSampleRate);
}

void SceneMemoProcessor::releaseResources()
{
    voiceAllocator.reset();
    fieldEngine.reset();
    gritSection.reset();
    motionFX.reset();
    sidechainModule.reset();
    reverbEffect.reset();
    delayEffect.reset();
    masterOutput.reset();
}

void SceneMemoProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    for (int ch = 0; ch < numChannels; ++ch)
        buffer.clear (ch, 0, numSamples);

    // Read all parameters once per block
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

    // ========================================================================
    // 1. Scene Engine (synthesis) → renders into main buffer
    // ========================================================================
    voiceAllocator.processBlock (buffer, midi, engineParams, bpm, ppqPosition);

    // ========================================================================
    // 2. Field Engine (granular/spectral) → renders into temp buffer, blend
    // ========================================================================
    if (fieldEngine.hasAudioLoaded())
    {
        // Render Field Engine into temp buffers
        juce::AudioBuffer<float> fieldBuffer (numChannels, numSamples);
        fieldBuffer.clear();

        FieldEngineParams fieldParams;
        fieldParams.level = 0.8f;
        fieldEngine.renderBlock (fieldBuffer.getWritePointer (0),
                                 numChannels > 1 ? fieldBuffer.getWritePointer (1) : fieldBuffer.getWritePointer (0),
                                 numSamples, fieldParams);

        // Blend Scene + Field using the blend matrix
        // For now, additive blend (full Scene + Field)
        for (int ch = 0; ch < numChannels; ++ch)
            buffer.addFrom (ch, 0, fieldBuffer, ch, 0, numSamples);
    }

    // ========================================================================
    // 3. Processing Chain: Grit → Motion FX → Sidechain → Space
    // ========================================================================

    float* left = buffer.getWritePointer (0);
    float* right = numChannels > 1 ? buffer.getWritePointer (1) : left;

    // Grit Section (Tape + Vinyl + BitCrush + Decade)
    {
        GritParams gritParams;
        gritParams.decade = 1.0f; // TODO: expose as parameter
        gritSection.processBlock (left, right, numSamples, gritParams);
    }

    // Motion FX (Tremolo, Phaser, Chorus, Filter Sweep)
    {
        MotionFXParams motionParams;
        motionFX.processBlock (left, right, numSamples, motionParams, bpm);
    }

    // Sidechain
    {
        SidechainParams scParams;
        sidechainModule.processBlock (left, right, numSamples, scParams);
    }

    // Reverb
    {
        ReverbParams reverbParams;
        reverbParams.mix = 0.15f; // subtle default
        reverbParams.size = 0.5f;
        reverbParams.damping = 0.5f;
        reverbEffect.processBlock (left, right, numSamples, reverbParams);
    }

    // Delay
    {
        DelayParams delayParams;
        delayEffect.processBlock (left, right, numSamples, delayParams, bpm);
    }

    // ========================================================================
    // 4. Master Output (Width, EQ, Limiter)
    // ========================================================================
    {
        MasterOutputParams masterParams;
        masterParams.dryWet = engineParams.dryWet;
        masterParams.limiterEnabled = true;
        masterOutput.processBlock (left, right, numSamples, masterParams);
    }

    // ========================================================================
    // 5. Smoothed Master Volume
    // ========================================================================
    masterVolSmoothed.updateTarget();
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
