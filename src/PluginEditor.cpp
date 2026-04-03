#include "PluginEditor.h"
#include "engine/ParameterLayout.h"
#include "util/Constants.h"

namespace scenememo {

// ============================================================================
// Helper methods
// ============================================================================

void SceneMemoEditor::setupSlider (ParamControl& pc, const juce::String& labelText,
                                   const juce::String& paramId,
                                   juce::AudioProcessorValueTreeState& apvts,
                                   juce::Component* parent)
{
    pc.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    pc.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 55, 16);
    parent->addAndMakeVisible (pc.slider);

    pc.label.setText (labelText, juce::dontSendNotification);
    pc.label.setJustificationType (juce::Justification::centred);
    pc.label.setFont (juce::FontOptions (11.0f));
    parent->addAndMakeVisible (pc.label);

    pc.attachment = std::make_unique<SliderAttachment> (apvts, paramId, pc.slider);
}

void SceneMemoEditor::setupCombo (ChoiceControl& cc, const juce::String& labelText,
                                  const juce::String& paramId,
                                  juce::AudioProcessorValueTreeState& apvts,
                                  juce::Component* parent)
{
    parent->addAndMakeVisible (cc.combo);
    cc.label.setText (labelText, juce::dontSendNotification);
    cc.label.setJustificationType (juce::Justification::centred);
    cc.label.setFont (juce::FontOptions (11.0f));
    parent->addAndMakeVisible (cc.label);

    cc.attachment = std::make_unique<ComboBoxAttachment> (apvts, paramId, cc.combo);
}

// ============================================================================
// OscTab
// ============================================================================

SceneMemoEditor::OscTab::OscTab (juce::AudioProcessorValueTreeState& apvts, int i)
{
    auto id = [i] (const char* s) { return makeParamId ("osc", i, s); };

    setupCombo (type, "Type", id ("type"), apvts, this);
    juce::String wfId = (i == 1) ? param::kOscWaveform : id ("waveform");
    setupCombo (waveform, "Waveform", wfId, apvts, this);

    juce::String lvlId = (i == 1) ? param::kOscLevel : id ("level");
    juce::String tuneId = (i == 1) ? param::kOscTune : id ("tune");
    juce::String fineId = (i == 1) ? param::kOscFine : id ("fine");

    setupSlider (level, "Level", lvlId, apvts, this);
    setupSlider (pan, "Pan", id ("pan"), apvts, this);
    setupSlider (tune, "Tune", tuneId, apvts, this);
    setupSlider (fine, "Fine", fineId, apvts, this);
    setupSlider (octave, "Octave", id ("octave"), apvts, this);
    setupSlider (wtPos, "WT Pos", id ("wt_pos"), apvts, this);
    setupSlider (unison, "Unison", id ("unison"), apvts, this);
    setupSlider (spread, "Spread", id ("unison_spread"), apvts, this);
}

void SceneMemoEditor::OscTab::resized()
{
    auto area = getLocalBounds().reduced (10);
    int knobW = 70, knobH = 70, labelH = 16, gap = 5;

    // Combos on top
    auto topRow = area.removeFromTop (40);
    type.label.setBounds (topRow.removeFromLeft (40));
    type.combo.setBounds (topRow.removeFromLeft (120));
    topRow.removeFromLeft (20);
    waveform.label.setBounds (topRow.removeFromLeft (60));
    waveform.combo.setBounds (topRow.removeFromLeft (120));

    area.removeFromTop (10);

    // Knobs
    int x = 10, y = area.getY();
    auto placeKnob = [&] (ParamControl& pc) {
        pc.label.setBounds (x, y, knobW, labelH);
        pc.slider.setBounds (x, y + labelH + gap, knobW, knobH);
        x += knobW + gap;
    };

    placeKnob (level);
    placeKnob (pan);
    placeKnob (tune);
    placeKnob (fine);
    placeKnob (octave);
    placeKnob (wtPos);
    placeKnob (unison);
    placeKnob (spread);
}

// ============================================================================
// FilterTab
// ============================================================================

SceneMemoEditor::FilterTab::FilterTab (juce::AudioProcessorValueTreeState& apvts)
{
    setupCombo (type1, "F1 Type", makeParamId ("filt", 1, "type"), apvts, this);
    setupSlider (cutoff1, "Cutoff", makeParamId ("filt", 1, "cutoff"), apvts, this);
    setupSlider (reso1, "Reso", makeParamId ("filt", 1, "reso"), apvts, this);
    setupSlider (drive1, "Drive", makeParamId ("filt", 1, "drive"), apvts, this);
    setupSlider (keyTrack1, "KeyTrk", makeParamId ("filt", 1, "keytrack"), apvts, this);
    setupSlider (envAmt1, "EnvAmt", makeParamId ("filt", 1, "env_amt"), apvts, this);

    setupCombo (type2, "F2 Type", makeParamId ("filt", 2, "type"), apvts, this);
    setupSlider (cutoff2, "Cutoff", makeParamId ("filt", 2, "cutoff"), apvts, this);
    setupSlider (reso2, "Reso", makeParamId ("filt", 2, "reso"), apvts, this);
    setupSlider (drive2, "Drive", makeParamId ("filt", 2, "drive"), apvts, this);
    setupSlider (keyTrack2, "KeyTrk", makeParamId ("filt", 2, "keytrack"), apvts, this);
    setupSlider (envAmt2, "EnvAmt", makeParamId ("filt", 2, "env_amt"), apvts, this);

    setupCombo (routing, "Routing", "filt_routing", apvts, this);
}

void SceneMemoEditor::FilterTab::resized()
{
    auto area = getLocalBounds().reduced (10);
    int knobW = 65, knobH = 65, labelH = 16, gap = 4;

    // Filter 1 row
    auto row1 = area.removeFromTop (120);
    int x = 10, y = row1.getY();
    type1.label.setBounds (x, y, 50, labelH);
    type1.combo.setBounds (x + 50, y, 100, 22);
    y += 28;
    auto placeKnob = [&] (ParamControl& pc) {
        pc.label.setBounds (x, y, knobW, labelH);
        pc.slider.setBounds (x, y + labelH + gap, knobW, knobH);
        x += knobW + gap;
    };
    placeKnob (cutoff1); placeKnob (reso1); placeKnob (drive1);
    placeKnob (keyTrack1); placeKnob (envAmt1);

    // Filter 2 row
    area.removeFromTop (10);
    x = 10; y = area.getY();
    type2.label.setBounds (x, y, 50, labelH);
    type2.combo.setBounds (x + 50, y, 100, 22);
    y += 28;
    placeKnob (cutoff2); placeKnob (reso2); placeKnob (drive2);
    placeKnob (keyTrack2); placeKnob (envAmt2);

    // Routing
    routing.label.setBounds (450, 10, 60, labelH);
    routing.combo.setBounds (450, 28, 100, 22);
}

// ============================================================================
// EnvTab
// ============================================================================

SceneMemoEditor::EnvTab::EnvTab (juce::AudioProcessorValueTreeState& apvts, int i)
{
    auto id = [i] (const char* s) { return makeParamId ("env", i, s); };

    juce::String aId = (i == 1) ? juce::String (param::kEnvAttack) : id ("attack");
    juce::String hId = (i == 1) ? juce::String (param::kEnvHold) : id ("hold");
    juce::String dId = (i == 1) ? juce::String (param::kEnvDecay) : id ("decay");
    juce::String sId = (i == 1) ? juce::String (param::kEnvSustain) : id ("sustain");
    juce::String rId = (i == 1) ? juce::String (param::kEnvRelease) : id ("release");

    setupSlider (attack, "Attack", aId, apvts, this);
    setupSlider (hold, "Hold", hId, apvts, this);
    setupSlider (decay, "Decay", dId, apvts, this);
    setupSlider (sustain, "Sustain", sId, apvts, this);
    setupSlider (release, "Release", rId, apvts, this);

    setupSlider (attackCurve, "Atk Curve", id ("attack_curve"), apvts, this);
    setupSlider (decayCurve, "Dec Curve", id ("decay_curve"), apvts, this);
    setupSlider (releaseCurve, "Rel Curve", id ("release_curve"), apvts, this);
}

void SceneMemoEditor::EnvTab::resized()
{
    int knobW = 70, knobH = 70, labelH = 16, gap = 5;
    int x = 10, y = 10;

    auto placeKnob = [&] (ParamControl& pc) {
        pc.label.setBounds (x, y, knobW, labelH);
        pc.slider.setBounds (x, y + labelH + gap, knobW, knobH);
        x += knobW + gap;
    };

    placeKnob (attack); placeKnob (hold); placeKnob (decay);
    placeKnob (sustain); placeKnob (release);

    x = 10; y += knobH + labelH + gap + 15;
    placeKnob (attackCurve); placeKnob (decayCurve); placeKnob (releaseCurve);
}

// ============================================================================
// MasterTab
// ============================================================================

SceneMemoEditor::MasterTab::MasterTab (juce::AudioProcessorValueTreeState& apvts)
{
    setupSlider (masterVol, "Master", param::kMasterVol, apvts, this);
    setupSlider (dryWet, "Dry/Wet", param::kDryWet, apvts, this);
    setupSlider (macro1, "Macro 1", "macro1", apvts, this);
    setupSlider (macro2, "Macro 2", "macro2", apvts, this);
    setupSlider (macro3, "Macro 3", "macro3", apvts, this);
    setupSlider (macro4, "Macro 4", "macro4", apvts, this);

    bypassButton.setButtonText ("Bypass");
    addAndMakeVisible (bypassButton);
    bypassAtt = std::make_unique<ButtonAttachment> (apvts, param::kBypass, bypassButton);
}

void SceneMemoEditor::MasterTab::resized()
{
    int knobW = 70, knobH = 70, labelH = 16, gap = 5;
    int x = 10, y = 10;

    auto placeKnob = [&] (ParamControl& pc) {
        pc.label.setBounds (x, y, knobW, labelH);
        pc.slider.setBounds (x, y + labelH + gap, knobW, knobH);
        x += knobW + gap;
    };

    placeKnob (masterVol); placeKnob (dryWet);
    x += 20;
    placeKnob (macro1); placeKnob (macro2); placeKnob (macro3); placeKnob (macro4);

    bypassButton.setBounds (10, y + knobH + labelH + gap + 10, 100, 25);
}

// ============================================================================
// Main Editor
// ============================================================================

SceneMemoEditor::SceneMemoEditor (SceneMemoProcessor& p)
    : AudioProcessorEditor (p), processor (p)
{
    setSize (800, 450);

    auto& apvts = processor.getAPVTS();

    oscTab1 = std::make_unique<OscTab> (apvts, 1);
    oscTab2 = std::make_unique<OscTab> (apvts, 2);
    oscTab3 = std::make_unique<OscTab> (apvts, 3);
    oscTab4 = std::make_unique<OscTab> (apvts, 4);
    filterTab = std::make_unique<FilterTab> (apvts);
    envTab1 = std::make_unique<EnvTab> (apvts, 1);
    envTab2 = std::make_unique<EnvTab> (apvts, 2);
    envTab3 = std::make_unique<EnvTab> (apvts, 3);
    envTab4 = std::make_unique<EnvTab> (apvts, 4);
    masterTab = std::make_unique<MasterTab> (apvts);

    auto tabColor = juce::Colour (0xff2a2a3e);
    tabs.addTab ("Osc 1", tabColor, oscTab1.get(), false);
    tabs.addTab ("Osc 2", tabColor, oscTab2.get(), false);
    tabs.addTab ("Osc 3", tabColor, oscTab3.get(), false);
    tabs.addTab ("Osc 4", tabColor, oscTab4.get(), false);
    tabs.addTab ("Filter", tabColor, filterTab.get(), false);
    tabs.addTab ("Env 1", tabColor, envTab1.get(), false);
    tabs.addTab ("Env 2", tabColor, envTab2.get(), false);
    tabs.addTab ("Env 3", tabColor, envTab3.get(), false);
    tabs.addTab ("Env 4", tabColor, envTab4.get(), false);
    tabs.addTab ("Master", tabColor, masterTab.get(), false);

    addAndMakeVisible (tabs);
}

SceneMemoEditor::~SceneMemoEditor() = default;

void SceneMemoEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1a1a2e));

    g.setColour (juce::Colour (0xffe0c097));
    g.setFont (juce::FontOptions (20.0f));
    g.drawText ("SceneMemo", 10, 5, 200, 25, juce::Justification::left);
}

void SceneMemoEditor::resized()
{
    tabs.setBounds (getLocalBounds().withTrimmedTop (30));
}

} // namespace scenememo
