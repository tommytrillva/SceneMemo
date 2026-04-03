#include "mixer/SceneMorph.h"
#include <cmath>

namespace scenememo {

void SceneMorph::captureSceneA (const juce::AudioProcessorValueTreeState& apvts)
{
    captureScene (apvts, sceneA);
    sceneAValid = true;
}

void SceneMorph::captureSceneB (const juce::AudioProcessorValueTreeState& apvts)
{
    captureScene (apvts, sceneB);
    sceneBValid = true;
}

void SceneMorph::captureScene (const juce::AudioProcessorValueTreeState& apvts,
                                std::vector<ParamSnapshot>& scene)
{
    scene.clear();
    auto& tree = apvts.state;

    for (int i = 0; i < apvts.processor.getParameters().size(); ++i)
    {
        if (auto* param = dynamic_cast<juce::RangedAudioParameter*> (apvts.processor.getParameters()[i]))
        {
            ParamSnapshot snap;
            snap.id = param->getParameterID();
            snap.value = param->getValue(); // normalized 0-1
            scene.push_back (snap);
        }
    }
}

void SceneMorph::applyMorph (juce::AudioProcessorValueTreeState& apvts,
                              const SceneMorphParams& params)
{
    if (! sceneAValid || ! sceneBValid)
        return;

    float pos = shapeMorph (std::clamp (params.morphPosition, 0.0f, 1.0f), params.curve);

    for (size_t i = 0; i < sceneA.size() && i < sceneB.size(); ++i)
    {
        const auto& a = sceneA[i];
        const auto& b = sceneB[i];

        if (a.id != b.id)
            continue;

        if (isParameterLocked (a.id))
            continue;

        float morphedValue = a.value + (b.value - a.value) * pos;

        if (auto* param = apvts.getParameter (a.id))
            param->setValueNotifyingHost (morphedValue);
    }
}

void SceneMorph::lockParameter (const juce::String& paramId)
{
    lockedParams.insert (paramId);
}

void SceneMorph::unlockParameter (const juce::String& paramId)
{
    lockedParams.erase (paramId);
}

bool SceneMorph::isParameterLocked (const juce::String& paramId) const
{
    return lockedParams.count (paramId) > 0;
}

float SceneMorph::shapeMorph (float position, MorphCurve curve)
{
    switch (curve)
    {
        case MorphCurve::Linear:
            return position;

        case MorphCurve::SCurve:
            return position * position * (3.0f - 2.0f * position);

        case MorphCurve::Exponential:
            return position * position;

        case MorphCurve::Logarithmic:
            return std::sqrt (position);
    }
    return position;
}

} // namespace scenememo
