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
                                   float oscLevel, float tuneSemitones, float fineCents,
                                   int waveformIndex, const AHDSREnvelope::Parameters& envParams)
{
    // Update envelope parameters for all voices (read once per block from APVTS)
    for (auto& voice : voices)
        voice.setEnvelopeParameters (envParams);

    // Sample-accurate MIDI processing
    int currentSample = 0;
    const int totalSamples = buffer.getNumSamples();

    for (const auto metadata : midi)
    {
        const int eventSample = metadata.samplePosition;

        // Render voices up to this MIDI event
        if (eventSample > currentSample)
        {
            renderVoices (buffer, currentSample, eventSample - currentSample,
                          oscLevel, tuneSemitones, fineCents);
            currentSample = eventSample;
        }

        handleMidiEvent (metadata.getMessage(), waveformIndex);
    }

    // Render remaining samples after last MIDI event
    if (currentSample < totalSamples)
        renderVoices (buffer, currentSample, totalSamples - currentSample,
                      oscLevel, tuneSemitones, fineCents);

    // Increment age counters
    for (auto& voice : voices)
    {
        if (voice.isActive())
            voice.incrementAge();
    }
}

void VoiceAllocator::handleMidiEvent (const juce::MidiMessage& msg, int waveformIndex)
{
    if (msg.isNoteOn())
    {
        int note = msg.getNoteNumber();
        float velocity = msg.getFloatVelocity();

        // Check if this note is already playing (retrigger)
        Voice* existing = findVoicePlayingNote (note);
        if (existing != nullptr)
        {
            int wtIdx = std::clamp (waveformIndex, 0, static_cast<int> (Waveform::NumWaveforms) - 1);
            existing->noteOn (note, velocity, wavetables[static_cast<size_t> (wtIdx)]);
            return;
        }

        // Allocate a new voice
        Voice* voice = findFreeVoice();
        if (voice == nullptr)
            voice = findVoiceToSteal();

        if (voice != nullptr)
        {
            int wtIdx = std::clamp (waveformIndex, 0, static_cast<int> (Waveform::NumWaveforms) - 1);
            voice->noteOn (note, velocity, wavetables[static_cast<size_t> (wtIdx)]);
        }
    }
    else if (msg.isNoteOff())
    {
        Voice* voice = findVoicePlayingNote (msg.getNoteNumber());
        if (voice != nullptr)
            voice->noteOff();
    }
    else if (msg.isAllNotesOff() || msg.isAllSoundOff())
    {
        reset();
    }
}

void VoiceAllocator::renderVoices (juce::AudioBuffer<float>& buffer, int startSample, int numSamples,
                                   float oscLevel, float tuneSemitones, float fineCents)
{
    auto* leftChannel  = buffer.getWritePointer (0) + startSample;
    auto* rightChannel = buffer.getNumChannels() > 1
                         ? buffer.getWritePointer (1) + startSample
                         : leftChannel;

    for (auto& voice : voices)
    {
        if (voice.isActive())
            voice.renderBlock (leftChannel, rightChannel, numSamples,
                               oscLevel, tuneSemitones, fineCents, sampleRate);
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

    // Prefer voices in Release state
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

    // No voices in release — steal oldest active voice
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
