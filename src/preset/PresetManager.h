#pragma once

#include "preset/PresetData.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <functional>

namespace scenememo {

class PresetManager
{
public:
    PresetManager (juce::AudioProcessorValueTreeState& apvts);

    // Directories
    juce::File getUserPresetsDirectory() const;
    juce::File getFactoryPresetsDirectory() const;
    juce::File getAudioDirectory() const;

    // Save/Load
    bool savePreset (const PresetMetadata& metadata);
    bool loadPreset (const juce::File& file);
    bool loadPreset (int index); // by index in scanned list

    // Scanning
    void scanPresets();
    const std::vector<PresetData>& getPresets() const { return presets; }
    int getNumPresets() const { return static_cast<int> (presets.size()); }

    // Navigation
    void loadNextPreset();
    void loadPreviousPreset();
    int getCurrentPresetIndex() const { return currentIndex; }
    juce::String getCurrentPresetName() const;

    // Favorites
    void toggleFavorite (int index);
    std::vector<int> getFavorites() const;

    // Recent
    const std::vector<juce::String>& getRecentPresets() const { return recentPresets; }

    // Filter by category
    std::vector<int> getPresetsInCategory (PresetCategory category) const;

    // A/B comparison
    void captureA();
    void captureB();
    void recallA();
    void recallB();
    bool isCompareActive() const { return compareActive; }

    // Callback when preset changes
    std::function<void()> onPresetChanged;

private:
    juce::AudioProcessorValueTreeState& apvts;
    std::vector<PresetData> presets;
    int currentIndex = -1;

    std::vector<juce::String> recentPresets;
    static constexpr int kMaxRecent = 20;

    // A/B comparison state
    juce::ValueTree compareStateA;
    juce::ValueTree compareStateB;
    bool compareActive = false;

    void addRecent (const juce::String& name);
};

} // namespace scenememo
