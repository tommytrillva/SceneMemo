#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

namespace scenememo {

class SceneMemoEditor : public juce::AudioProcessorEditor
{
public:
    explicit SceneMemoEditor (SceneMemoProcessor&);
    ~SceneMemoEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    SceneMemoProcessor& processor;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    // Controls
    juce::Slider masterVolSlider, oscLevelSlider, tuneSlider, fineSlider, dryWetSlider;
    juce::Slider attackSlider, holdSlider, decaySlider, sustainSlider, releaseSlider;
    juce::ComboBox waveformBox;
    juce::ToggleButton bypassButton;

    // Labels
    juce::Label masterVolLabel, oscLevelLabel, tuneLabel, fineLabel, dryWetLabel;
    juce::Label attackLabel, holdLabel, decayLabel, sustainLabel, releaseLabel;
    juce::Label waveformLabel;

    // Attachments (must be destroyed before the controls they reference)
    std::unique_ptr<SliderAttachment> masterVolAtt, oscLevelAtt, tuneAtt, fineAtt, dryWetAtt;
    std::unique_ptr<SliderAttachment> attackAtt, holdAtt, decayAtt, sustainAtt, releaseAtt;
    std::unique_ptr<ComboBoxAttachment> waveformAtt;
    std::unique_ptr<ButtonAttachment> bypassAtt;

    void setupSlider (juce::Slider& slider, juce::Label& label, const juce::String& labelText,
                      std::unique_ptr<SliderAttachment>& attachment, const juce::String& paramId);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneMemoEditor)
};

} // namespace scenememo
