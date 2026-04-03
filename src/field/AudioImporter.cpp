#include "field/AudioImporter.h"

namespace scenememo {

AudioImporter::AudioImporter()
{
    formatManager.registerBasicFormats(); // WAV, AIFF, FLAC, OGG, MP3
}

AudioImporter::~AudioImporter() = default;

bool AudioImporter::isFormatSupported (const juce::File& file) const
{
    auto ext = file.getFileExtension().toLowerCase();
    return ext == ".wav" || ext == ".aiff" || ext == ".aif"
        || ext == ".flac" || ext == ".ogg" || ext == ".mp3"
        || ext == ".m4a";
}

juce::StringArray AudioImporter::getSupportedExtensions() const
{
    return { ".wav", ".aiff", ".aif", ".flac", ".ogg", ".mp3", ".m4a" };
}

std::unique_ptr<ImportedAudio> AudioImporter::loadFile (const juce::File& file, double targetSampleRate)
{
    if (! file.existsAsFile())
        return nullptr;

    if (file.getSize() > kMaxFileSizeBytes)
        return nullptr;

    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
    if (reader == nullptr)
        return nullptr;

    auto result = std::make_unique<ImportedAudio>();
    result->sampleRate = reader->sampleRate;
    result->numChannels = static_cast<int> (reader->numChannels);
    result->fileName = file.getFileName();
    result->filePath = file.getFullPathName();

    // Truncate to max length
    int64_t maxSamples = static_cast<int64_t> (kMaxFileLengthSeconds * reader->sampleRate);
    int64_t samplesToRead = std::min (static_cast<int64_t> (reader->lengthInSamples), maxSamples);

    // Read audio into buffer (mono mix-down for simplicity)
    juce::AudioBuffer<float> rawBuffer (static_cast<int> (reader->numChannels),
                                         static_cast<int> (samplesToRead));
    reader->read (&rawBuffer, 0, static_cast<int> (samplesToRead), 0, true, true);

    // Mix to mono
    juce::AudioBuffer<float> monoBuffer (1, static_cast<int> (samplesToRead));
    monoBuffer.clear();
    for (int ch = 0; ch < rawBuffer.getNumChannels(); ++ch)
        monoBuffer.addFrom (0, 0, rawBuffer, ch, 0, static_cast<int> (samplesToRead),
                            1.0f / static_cast<float> (rawBuffer.getNumChannels()));

    // Resample to target sample rate if needed
    if (std::abs (reader->sampleRate - targetSampleRate) > 1.0)
    {
        double ratio = targetSampleRate / reader->sampleRate;
        int newLength = static_cast<int> (static_cast<double> (samplesToRead) * ratio);

        juce::AudioBuffer<float> resampledBuffer (1, newLength);
        resampledBuffer.clear();

        // Simple linear interpolation resampling
        const float* src = monoBuffer.getReadPointer (0);
        float* dst = resampledBuffer.getWritePointer (0);

        for (int i = 0; i < newLength; ++i)
        {
            double srcPos = static_cast<double> (i) / ratio;
            int srcIdx = static_cast<int> (srcPos);
            float frac = static_cast<float> (srcPos - srcIdx);

            if (srcIdx + 1 < static_cast<int> (samplesToRead))
                dst[i] = src[srcIdx] * (1.0f - frac) + src[srcIdx + 1] * frac;
            else if (srcIdx < static_cast<int> (samplesToRead))
                dst[i] = src[srcIdx];
        }

        result->buffer = std::move (resampledBuffer);
        result->sampleRate = targetSampleRate;
        result->numSamples = newLength;
    }
    else
    {
        result->buffer = std::move (monoBuffer);
        result->numSamples = static_cast<int> (samplesToRead);
    }

    result->numChannels = 1;
    addRecentCapture (file.getFullPathName());
    return result;
}

void AudioImporter::addRecentCapture (const juce::String& path)
{
    // Remove if already in list
    recentCaptures.erase (
        std::remove (recentCaptures.begin(), recentCaptures.end(), path),
        recentCaptures.end());

    recentCaptures.insert (recentCaptures.begin(), path);

    while (static_cast<int> (recentCaptures.size()) > kMaxRecentCaptures)
        recentCaptures.pop_back();
}

} // namespace scenememo
