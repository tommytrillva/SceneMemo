#include "engine/VoiceAllocator.h"
#include <algorithm>

namespace scenememo {

VoiceAllocator::VoiceAllocator()
{
    buildWavetables();
}

void VoiceAllocator::buildWavetables()
{
    wavetables[static_cast<size_t> (Waveform::Sine)]     = WavetableFactory::createSine();
    wavetables[static_cast<size_t> (Waveform::Saw)]      = WavetableFactory::createSaw();
    wavetables[static_cast<size_t> (Waveform::Square)]   = WavetableFactory::createSquare();
    wavetables[static_cast<size_t> (Waveform::Triangle)] = WavetableFactory::createTriangle();
}

void VoiceAllocator::prepareToPlay (float sr, int bs)
{
    sampleRate = sr;
    blockSize = bs;

    for (auto& voice : voices)
        voice.prepareToPlay (sr, bs);
}

void VoiceAllocator::reset()
{
    for (auto& voice : voices)
        voice.reset();
}

void VoiceAllocator::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi,
                                   const EngineParams& params, float bpm, double ppqPosition)
{
    int currentSample = 0;
    const int totalSamples = buffer.getNumSamples();

    for (const auto metadata : midi)
    {
        const int eventSample = metadata.samplePosition;

        if (eventSample > currentSample)
        {
            renderVoices (buffer, currentSample, eventSample - currentSample,
                          params, bpm, ppqPosition);
            currentSample = eventSample;
        }

        handleMidiEvent (metadata.getMessage(), params);
    }

    if (currentSample < totalSamples)
        renderVoices (buffer, currentSample, totalSamples - currentSample,
                      params, bpm, ppqPosition);

    for (auto& voice : voices)
    {
        if (voice.isActive())
            voice.incrementAge();
    }
}

void VoiceAllocator::handleMidiEvent (const juce::MidiMessage& msg, const EngineParams& params)
{
    if (msg.isNoteOn())
    {
        int note = msg.getNoteNumber();
        float velocity = msg.getFloatVelocity();

        Voice* existing = findVoicePlayingNote (note);
        if (existing != nullptr)
        {
            existing->noteOn (note, velocity, params, wavetables);
            return;
        }

        Voice* voice = findFreeVoice();
        if (voice == nullptr)
            voice = findVoiceToSteal();

        if (voice != nullptr)
            voice->noteOn (note, velocity, params, wavetables);
    }
    else if (msg.isNoteOff())
    {
        Voice* voice = findVoicePlayingNote (msg.getNoteNumber());
        if (voice != nullptr)
            voice->noteOff();
    }
    else if (msg.isControllerOfType (1)) // Mod wheel
    {
        float value = static_cast<float> (msg.getControllerValue()) / 127.0f;
        for (auto& voice : voices)
            voice.setModWheel (value);
    }
    else if (msg.isChannelPressure())
    {
        float value = static_cast<float> (msg.getChannelPressureValue()) / 127.0f;
        for (auto& voice : voices)
            voice.setAftertouch (value);
    }
    else if (msg.isAllNotesOff() || msg.isAllSoundOff())
    {
        reset();
    }
}

void VoiceAllocator::renderVoices (juce::AudioBuffer<float>& buffer, int startSample, int numSamples,
                                   const EngineParams& params, float bpm, double ppqPosition)
{
    auto* leftChannel  = buffer.getWritePointer (0) + startSample;
    auto* rightChannel = buffer.getNumChannels() > 1
                         ? buffer.getWritePointer (1) + startSample
                         : leftChannel;

    for (auto& voice : voices)
    {
        if (voice.isActive())
            voice.renderBlock (leftChannel, rightChannel, numSamples,
                               params, modMatrix, wavetables,
                               sampleRate, bpm, ppqPosition);
    }
}

Voice* VoiceAllocator::findFreeVoice()
{
    for (auto& voice : voices)
    {
        if (! voice.isActive())
            return &voice;
    }
    return nullptr;
}

Voice* VoiceAllocator::findVoiceToSteal()
{
    Voice* best = nullptr;
    int bestAge = -1;

    for (auto& voice : voices)
    {
        if (voice.getEnvelopeState() == AHDSREnvelope::State::Release
            && voice.getAge() > bestAge)
        {
            best = &voice;
            bestAge = voice.getAge();
        }
    }

    if (best != nullptr)
    {
        best->reset();
        return best;
    }

    for (auto& voice : voices)
    {
        if (voice.getAge() > bestAge)
        {
            best = &voice;
            bestAge = voice.getAge();
        }
    }

    if (best != nullptr)
        best->reset();

    return best;
}

Voice* VoiceAllocator::findVoicePlayingNote (int midiNote)
{
    for (auto& voice : voices)
    {
        if (voice.isActive() && voice.getCurrentNote() == midiNote)
            return &voice;
    }
    return nullptr;
}

} // namespace scenememo
