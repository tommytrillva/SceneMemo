#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <string>

namespace scenememo {

// Helper to generate parameter IDs like "osc2_level"
inline juce::String makeParamId (const char* prefix, int index, const char* suffix)
{
    return juce::String (prefix) + juce::String (index) + "_" + juce::String (suffix);
}

namespace params {

void addGlobalParams (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& params);
void addOscParams (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& params, int oscIndex);
void addFilterParams (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& params, int filtIndex);
void addFilterRoutingParam (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& params);
void addEnvParams (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& params, int envIndex);
void addLfoParams (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& params, int lfoIndex);
void addModSlotParams (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& params, int slotIndex);
void addMacroParams (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& params);

juce::AudioProcessorValueTreeState::ParameterLayout createFullLayout();

} // namespace params
} // namespace scenememo
