#include "../TestFramework.h"
#include "engine/LFO.h"

using namespace scenememo;

TEST(lfo_sine_range)
{
    LFO lfo;
    lfo.prepareToPlay(44100.0f);

    float minVal = 1.0f, maxVal = -1.0f;
    for (int i = 0; i < 44100; ++i)
    {
        float val = lfo.processSample(LFOShape::Sine, 1.0f, false, 0, 0.0f, 0.0f, 120.0f, 0.0);
        minVal = std::min(minVal, val);
        maxVal = std::max(maxVal, val);
    }

    EXPECT(maxVal > 0.9f);
    EXPECT(minVal < -0.9f);
    return true;
}

TEST(lfo_square_5050)
{
    LFO lfo;
    lfo.prepareToPlay(44100.0f);

    int pos = 0, neg = 0;
    for (int i = 0; i < 44100; ++i)
    {
        float val = lfo.processSample(LFOShape::Square, 1.0f, false, 0, 0.0f, 0.0f, 120.0f, 0.0);
        if (val > 0.0f) pos++; else neg++;
    }
    float ratio = static_cast<float>(pos) / static_cast<float>(pos + neg);
    EXPECT_NEAR(ratio, 0.5f, 0.05f);
    return true;
}

TEST(lfo_all_shapes_no_nan)
{
    LFO lfo;
    lfo.prepareToPlay(44100.0f);

    LFOShape shapes[] = {
        LFOShape::Sine, LFOShape::Triangle, LFOShape::SawUp,
        LFOShape::SawDown, LFOShape::Square, LFOShape::SampleHold,
        LFOShape::RandomSmooth
    };

    for (auto shape : shapes)
    {
        lfo.reset();
        for (int i = 0; i < 4410; ++i)
        {
            float val = lfo.processSample(shape, 2.0f, false, 0, 0.0f, 0.0f, 120.0f, 0.0);
            EXPECT(!std::isnan(val));
            EXPECT(!std::isinf(val));
        }
    }
    return true;
}

TEST(lfo_tempo_sync)
{
    LFO lfo;
    lfo.prepareToPlay(44100.0f);

    // At 120 BPM, 1/4 note sync (index 9) = 0.5s period = 2 Hz
    // Run for 2 full seconds to ensure multiple complete cycles
    float minVal = 1.0f, maxVal = -1.0f;
    for (int i = 0; i < 88200; ++i)
    {
        float val = lfo.processSample(LFOShape::Sine, 1.0f, true, 9, 0.0f, 0.0f, 120.0f, 0.0);
        minVal = std::min(minVal, val);
        maxVal = std::max(maxVal, val);
    }

    EXPECT(maxVal > 0.9f);
    EXPECT(minVal < -0.9f);
    return true;
}
