#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <memory>
#include <vector>
#include <functional>

namespace scenememo {

// Holds imported audio data with metadata.
struct ImportedAudio
{
    juce::AudioBuffer<float> buffer;
    double sampleRate = 44100.0;
    int numChannels = 1;
    int numSamples = 0;
    juce::String fileName;
    juce::String filePath;

    float getLengthSeconds() const
    {
        return (sampleRate > 0) ? static_cast<float> (numSamples) / static_cast<float> (sampleRate) : 0.0f;
    }
};

// Handles loading audio files in various formats into a buffer.
// Runs on a background thread — never call from audio thread.
class AudioImporter
{
public:
    AudioImporter();
    ~AudioImporter();

    // Load an audio file, resample to target sample rate if needed.
    // Returns nullptr on failure.
    std::unique_ptr<ImportedAudio> loadFile (const juce::File& file, double targetSampleRate);

    // Supported format check
    bool isFormatSupported (const juce::File& file) const;

    // Get list of supported extensions
    juce::StringArray getSupportedExtensions() const;

    // Max file length in seconds (longer files are truncated)
    static constexpr float kMaxFileLengthSeconds = 300.0f; // 5 minutes
    static constexpr int64_t kMaxFileSizeBytes = 100 * 1024 * 1024; // 100MB

    // Recent captures management
    void addRecentCapture (const juce::String& path);
    const std::vector<juce::String>& getRecentCaptures() const { return recentCaptures; }
    void clearRecentCaptures() { recentCaptures.clear(); }

private:
    juce::AudioFormatManager formatManager;
    std::vector<juce::String> recentCaptures;
    static constexpr int kMaxRecentCaptures = 20;
};

} // namespace scenememo
