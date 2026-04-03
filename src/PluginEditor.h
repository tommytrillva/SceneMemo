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

    // Tabs
    juce::TabbedComponent tabs { juce::TabbedButtonBar::TabsAtTop };

    // Helper to create a tab page with knobs for given parameters
    struct ParamControl
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<SliderAttachment> attachment;
    };

    struct ChoiceControl
    {
        juce::ComboBox combo;
        juce::Label label;
        std::unique_ptr<ComboBoxAttachment> attachment;
    };

    // Oscillator tab
    struct OscTab : public juce::Component
    {
        OscTab (juce::AudioProcessorValueTreeState& apvts, int oscIndex);
        void resized() override;

        ChoiceControl type, waveform;
        ParamControl level, pan, tune, fine, octave, wtPos, unison, spread;
    };

    // Filter tab
    struct FilterTab : public juce::Component
    {
        FilterTab (juce::AudioProcessorValueTreeState& apvts);
        void resized() override;

        ChoiceControl type1, type2, routing;
        ParamControl cutoff1, reso1, drive1, keyTrack1, envAmt1;
        ParamControl cutoff2, reso2, drive2, keyTrack2, envAmt2;
    };

    // Envelope tab
    struct EnvTab : public juce::Component
    {
        EnvTab (juce::AudioProcessorValueTreeState& apvts, int envIndex);
        void resized() override;

        ParamControl attack, hold, decay, sustain, release;
        ParamControl attackCurve, decayCurve, releaseCurve;
    };

    // Master / Global tab
    struct MasterTab : public juce::Component
    {
        MasterTab (juce::AudioProcessorValueTreeState& apvts);
        void resized() override;

        ParamControl masterVol, dryWet;
        ParamControl macro1, macro2, macro3, macro4;
        juce::ToggleButton bypassButton;
        std::unique_ptr<ButtonAttachment> bypassAtt;
    };

    // Field Engine tab
    struct FieldTab : public juce::Component, public juce::FileDragAndDropTarget
    {
        FieldTab (SceneMemoProcessor& proc);
        void resized() override;
        void paint (juce::Graphics&) override;
        bool isInterestedInFileDrag (const juce::StringArray& files) override;
        void filesDropped (const juce::StringArray& files, int x, int y) override;

        SceneMemoProcessor& processor;
        juce::TextButton loadButton { "Load Audio" };
        juce::TextButton recordButton { "Record" };
        juce::TextButton freezeButton { "Freeze" };
        juce::Label statusLabel;
    };

    // Owned tab components
    std::unique_ptr<OscTab> oscTab1, oscTab2, oscTab3, oscTab4;
    std::unique_ptr<FilterTab> filterTab;
    std::unique_ptr<EnvTab> envTab1, envTab2, envTab3, envTab4;
    std::unique_ptr<MasterTab> masterTab;
    std::unique_ptr<FieldTab> fieldTab;

    // Helper methods
    static void setupSlider (ParamControl& pc, const juce::String& labelText,
                             const juce::String& paramId,
                             juce::AudioProcessorValueTreeState& apvts,
                             juce::Component* parent);
    static void setupCombo (ChoiceControl& cc, const juce::String& labelText,
                            const juce::String& paramId,
                            juce::AudioProcessorValueTreeState& apvts,
                            juce::Component* parent);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneMemoEditor)
};

} // namespace scenememo
