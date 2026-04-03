#pragma once

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <vector>

namespace scenememo {

// Tonal Palette categories
enum class PresetCategory
{
    GoldenHour = 0,
    NightDrive,
    RooftopRain,
    CityFog,
    DesertHeat,
    NeonAlley,
    OceanFloor,
    MidnightStudio,
    FirstLight,
    StreetLevel,
    MemoryLane,
    TheCosmos,
    NumCategories
};

inline juce::String getCategoryName (PresetCategory cat)
{
    switch (cat)
    {
        case PresetCategory::GoldenHour:     return "Golden Hour";
        case PresetCategory::NightDrive:     return "Night Drive";
        case PresetCategory::RooftopRain:    return "Rooftop Rain";
        case PresetCategory::CityFog:        return "City Fog";
        case PresetCategory::DesertHeat:     return "Desert Heat";
        case PresetCategory::NeonAlley:      return "Neon Alley";
        case PresetCategory::OceanFloor:     return "Ocean Floor";
        case PresetCategory::MidnightStudio: return "Midnight Studio";
        case PresetCategory::FirstLight:     return "First Light";
        case PresetCategory::StreetLevel:    return "Street Level";
        case PresetCategory::MemoryLane:     return "Memory Lane";
        case PresetCategory::TheCosmos:      return "The Cosmos";
        default: return "Unknown";
    }
}

struct PresetMetadata
{
    juce::String name;
    juce::String author;
    PresetCategory category = PresetCategory::GoldenHour;
    juce::StringArray tags;
    juce::String description;
    juce::String sourceAudioPath;  // relative path to associated audio
    juce::String locationTag;
    juce::String dateCreated;
    juce::String dateModified;
    bool isFavorite = false;
};

// Represents a complete preset (metadata + parameter state)
struct PresetData
{
    PresetMetadata metadata;
    juce::ValueTree parameterState; // APVTS state tree

    // Serialize to JSON string (.smemo format)
    juce::String toJSON() const;

    // Deserialize from JSON string
    static PresetData fromJSON (const juce::String& jsonString);

    // Save/load to/from file
    bool saveToFile (const juce::File& file) const;
    static PresetData loadFromFile (const juce::File& file);
};

} // namespace scenememo
