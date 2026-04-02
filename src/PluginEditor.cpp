#include "PluginEditor.h"
#include "util/Constants.h"

namespace scenememo {

SceneMemoEditor::SceneMemoEditor (SceneMemoProcessor& p)
    : AudioProcessorEditor (p), processor (p)
{
    setSize (700, 400);

    auto& apvts = processor.getAPVTS();

    // --- Oscillator Section ---
    setupSlider (masterVolSlider, masterVolLabel, "Master", masterVolAtt, param::kMasterVol);
    setupSlider (oscLevelSlider, oscLevelLabel, "Osc Level", oscLevelAtt, param::kOscLevel);
    setupSlider (tuneSlider, tuneLabel, "Tune", tuneAtt, param::kOscTune);
    setupSlider (fineSlider, fineLabel, "Fine", fineAtt, param::kOscFine);
    setupSlider (dryWetSlider, dryWetLabel, "Dry/Wet", dryWetAtt, param::kDryWet);

    // Waveform combo
    waveformBox.addItemList ({ "Sine", "Saw", "Square", "Triangle" }, 1);
    addAndMakeVisible (waveformBox);
    waveformAtt = std::make_unique<ComboBoxAttachment> (apvts, param::kOscWaveform, waveformBox);
    waveformLabel.setText ("Waveform", juce::dontSendNotification);
    waveformLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (waveformLabel);

    // Bypass toggle
    bypassButton.setButtonText ("Bypass");
    addAndMakeVisible (bypassButton);
    bypassAtt = std::make_unique<ButtonAttachment> (apvts, param::kBypass, bypassButton);

    // --- Envelope Section ---
    setupSlider (attackSlider, attackLabel, "Attack", attackAtt, param::kEnvAttack);
    setupSlider (holdSlider, holdLabel, "Hold", holdAtt, param::kEnvHold);
    setupSlider (decaySlider, decayLabel, "Decay", decayAtt, param::kEnvDecay);
    setupSlider (sustainSlider, sustainLabel, "Sustain", sustainAtt, param::kEnvSustain);
    setupSlider (releaseSlider, releaseLabel, "Release", releaseAtt, param::kEnvRelease);
}

SceneMemoEditor::~SceneMemoEditor() = default;

void SceneMemoEditor::setupSlider (juce::Slider& slider, juce::Label& label,
                                   const juce::String& labelText,
                                   std::unique_ptr<SliderAttachment>& attachment,
                                   const juce::String& paramId)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
    addAndMakeVisible (slider);

    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (label);

    attachment = std::make_unique<SliderAttachment> (processor.getAPVTS(), paramId, slider);
}

void SceneMemoEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1a1a2e));

    // Title
    g.setColour (juce::Colour (0xffe0c097));
    g.setFont (juce::FontOptions (22.0f));
    g.drawText ("SceneMemo", getLocalBounds().removeFromTop (35), juce::Justification::centred);

    // Section labels
    g.setFont (juce::FontOptions (13.0f));
    g.setColour (juce::Colour (0xff8888aa));
    g.drawText ("OSCILLATOR", 10, 40, 400, 20, juce::Justification::left);
    g.drawText ("ENVELOPE", 10, 210, 400, 20, juce::Justification::left);
    g.drawText ("OUTPUT", 470, 40, 200, 20, juce::Justification::left);
}

void SceneMemoEditor::resized()
{
    const int knobW = 80;
    const int knobH = 80;
    const int labelH = 18;
    const int gap = 5;

    // --- Row 1: Oscillator controls (y=60) ---
    int x = 20;
    int y = 60;

    auto placeKnob = [&] (juce::Slider& slider, juce::Label& label) {
        label.setBounds (x, y, knobW, labelH);
        slider.setBounds (x, y + labelH + gap, knobW, knobH);
        x += knobW + gap;
    };

    // Waveform combo gets special layout
    waveformLabel.setBounds (x, y, 100, labelH);
    waveformBox.setBounds (x, y + labelH + gap, 100, 25);
    x += 105 + gap;

    placeKnob (oscLevelSlider, oscLevelLabel);
    placeKnob (tuneSlider, tuneLabel);
    placeKnob (fineSlider, fineLabel);

    // Output section (right side)
    x = 480;
    placeKnob (masterVolSlider, masterVolLabel);
    placeKnob (dryWetSlider, dryWetLabel);

    bypassButton.setBounds (480, y + labelH + knobH + gap + 10, 100, 25);

    // --- Row 2: Envelope controls (y=230) ---
    x = 20;
    y = 230;

    placeKnob (attackSlider, attackLabel);
    placeKnob (holdSlider, holdLabel);
    placeKnob (decaySlider, decayLabel);
    placeKnob (sustainSlider, sustainLabel);
    placeKnob (releaseSlider, releaseLabel);
}

} // namespace scenememo
