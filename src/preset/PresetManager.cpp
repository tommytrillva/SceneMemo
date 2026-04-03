#include "preset/PresetManager.h"
#include <algorithm>

namespace scenememo {

PresetManager::PresetManager (juce::AudioProcessorValueTreeState& apvts_)
    : apvts (apvts_)
{
}

juce::File PresetManager::getUserPresetsDirectory() const
{
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
        .getChildFile ("SceneMemo").getChildFile ("Presets");
}

juce::File PresetManager::getFactoryPresetsDirectory() const
{
    #if JUCE_MAC
        return juce::File ("/Library/Application Support/SceneMemo/Presets");
    #elif JUCE_WINDOWS
        return juce::File ("C:\\ProgramData\\SceneMemo\\Presets");
    #else
        return juce::File::getSpecialLocation (juce::File::commonApplicationDataDirectory)
            .getChildFile ("SceneMemo").getChildFile ("Presets");
    #endif
}

juce::File PresetManager::getAudioDirectory() const
{
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
        .getChildFile ("SceneMemo").getChildFile ("Audio");
}

bool PresetManager::savePreset (const PresetMetadata& metadata)
{
    auto dir = getUserPresetsDirectory();
    dir.createDirectory();

    PresetData preset;
    preset.metadata = metadata;
    preset.metadata.dateModified = juce::Time::getCurrentTime().toISO8601 (true);
    if (preset.metadata.dateCreated.isEmpty())
        preset.metadata.dateCreated = preset.metadata.dateModified;

    preset.parameterState = apvts.copyState();

    auto file = dir.getChildFile (metadata.name + ".smemo");
    bool success = preset.saveToFile (file);

    if (success)
        scanPresets(); // refresh list

    return success;
}

bool PresetManager::loadPreset (const juce::File& file)
{
    auto preset = PresetData::loadFromFile (file);
    if (! preset.parameterState.isValid())
        return false;

    apvts.replaceState (preset.parameterState);
    addRecent (preset.metadata.name);

    if (onPresetChanged)
        onPresetChanged();

    return true;
}

bool PresetManager::loadPreset (int index)
{
    if (index < 0 || index >= getNumPresets())
        return false;

    const auto& preset = presets[index];
    if (! preset.parameterState.isValid())
        return false;

    apvts.replaceState (preset.parameterState);
    currentIndex = index;
    addRecent (preset.metadata.name);

    if (onPresetChanged)
        onPresetChanged();

    return true;
}

void PresetManager::scanPresets()
{
    presets.clear();

    auto scanDir = [this] (const juce::File& dir)
    {
        if (! dir.isDirectory())
            return;

        for (const auto& entry : juce::RangedDirectoryIterator (dir, false, "*.smemo"))
        {
            auto preset = PresetData::loadFromFile (entry.getFile());
            if (preset.parameterState.isValid() || preset.metadata.name.isNotEmpty())
                presets.push_back (std::move (preset));
        }
    };

    scanDir (getFactoryPresetsDirectory());
    scanDir (getUserPresetsDirectory());

    // Sort by name
    std::sort (presets.begin(), presets.end(),
        [] (const PresetData& a, const PresetData& b) {
            return a.metadata.name.compareIgnoreCase (b.metadata.name) < 0;
        });
}

void PresetManager::loadNextPreset()
{
    if (presets.empty())
        return;
    currentIndex = (currentIndex + 1) % getNumPresets();
    loadPreset (currentIndex);
}

void PresetManager::loadPreviousPreset()
{
    if (presets.empty())
        return;
    currentIndex = (currentIndex - 1 + getNumPresets()) % getNumPresets();
    loadPreset (currentIndex);
}

juce::String PresetManager::getCurrentPresetName() const
{
    if (currentIndex >= 0 && currentIndex < getNumPresets())
        return presets[currentIndex].metadata.name;
    return "Init";
}

void PresetManager::toggleFavorite (int index)
{
    if (index >= 0 && index < getNumPresets())
        presets[index].metadata.isFavorite = ! presets[index].metadata.isFavorite;
}

std::vector<int> PresetManager::getFavorites() const
{
    std::vector<int> result;
    for (int i = 0; i < getNumPresets(); ++i)
        if (presets[i].metadata.isFavorite)
            result.push_back (i);
    return result;
}

std::vector<int> PresetManager::getPresetsInCategory (PresetCategory category) const
{
    std::vector<int> result;
    for (int i = 0; i < getNumPresets(); ++i)
        if (presets[i].metadata.category == category)
            result.push_back (i);
    return result;
}

void PresetManager::captureA()
{
    compareStateA = apvts.copyState();
}

void PresetManager::captureB()
{
    compareStateB = apvts.copyState();
    compareActive = true;
}

void PresetManager::recallA()
{
    if (compareStateA.isValid())
        apvts.replaceState (compareStateA);
}

void PresetManager::recallB()
{
    if (compareStateB.isValid())
        apvts.replaceState (compareStateB);
}

void PresetManager::addRecent (const juce::String& name)
{
    recentPresets.erase (
        std::remove (recentPresets.begin(), recentPresets.end(), name),
        recentPresets.end());
    recentPresets.insert (recentPresets.begin(), name);
    while (static_cast<int> (recentPresets.size()) > kMaxRecent)
        recentPresets.pop_back();
}

} // namespace scenememo
