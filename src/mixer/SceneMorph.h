#pragma once

#include <juce_data_structures/juce_data_structures.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <set>

namespace scenememo {

// Morph curve shapes
enum class MorphCurve { Linear, SCurve, Exponential, Logarithmic };

struct SceneMorphParams
{
    float morphPosition = 0.0f;  // 0 = Scene A, 1 = Scene B
    bool autoMorph = false;
    float morphBars = 4.0f;      // bars for auto morph
    bool pingPong = false;
    MorphCurve curve = MorphCurve::Linear;
};

// A/B scene snapshot and morphing system.
// Stores two complete parameter snapshots and crossfades between them.
class SceneMorph
{
public:
    SceneMorph() = default;

    // Capture current APVTS state as Scene A or B
    void captureSceneA (const juce::AudioProcessorValueTreeState& apvts);
    void captureSceneB (const juce::AudioProcessorValueTreeState& apvts);

    // Apply morphed state to APVTS based on current morph position
    void applyMorph (juce::AudioProcessorValueTreeState& apvts,
                     const SceneMorphParams& params);

    // Lock a parameter so it doesn't change during morph
    void lockParameter (const juce::String& paramId);
    void unlockParameter (const juce::String& paramId);
    bool isParameterLocked (const juce::String& paramId) const;

    bool hasSceneA() const { return sceneAValid; }
    bool hasSceneB() const { return sceneBValid; }

    // Apply a morph curve shape to a linear position
    static float shapeMorph (float position, MorphCurve curve);

private:
    struct ParamSnapshot
    {
        juce::String id;
        float value = 0.0f;
    };

    std::vector<ParamSnapshot> sceneA;
    std::vector<ParamSnapshot> sceneB;
    bool sceneAValid = false;
    bool sceneBValid = false;

    std::set<juce::String> lockedParams;

    void captureScene (const juce::AudioProcessorValueTreeState& apvts,
                       std::vector<ParamSnapshot>& scene);
};

} // namespace scenememo
