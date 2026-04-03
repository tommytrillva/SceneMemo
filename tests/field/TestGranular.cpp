#include "../TestFramework.h"
#include "field/GranularEngine.h"

using namespace scenememo;

TEST(granular_no_output_without_source)
{
    GranularEngine engine;
    engine.prepareToPlay(44100.0f);

    float outL[512] = {}, outR[512] = {};
    GranularParams params;
    params.grainDensity = 20.0f;
    engine.renderBlock(outL, outR, 512, params);

    float sum = 0.0f;
    for (int i = 0; i < 512; ++i)
        sum += std::abs(outL[i]) + std::abs(outR[i]);

    EXPECT_NEAR(sum, 0.0f, 0.0001f);
    return true;
}

TEST(granular_produces_output_with_source)
{
    GranularEngine engine;
    engine.prepareToPlay(44100.0f);

    juce::AudioBuffer<float> source(1, 44100);
    auto* data = source.getWritePointer(0);
    for (int i = 0; i < 44100; ++i)
        data[i] = std::sin(2.0f * 3.14159265f * 440.0f * i / 44100.0f);

    engine.setSourceBuffer(&source, 44100.0);

    float outL[4096] = {}, outR[4096] = {};
    GranularParams params;
    params.grainSize = 50.0f;
    params.grainDensity = 30.0f;
    params.grainPosition = 0.5f;
    params.mode = GranularMode::Scrub;
    engine.renderBlock(outL, outR, 4096, params);

    float sum = 0.0f;
    for (int i = 0; i < 4096; ++i)
        sum += std::abs(outL[i]);

    EXPECT(sum > 0.01f);
    return true;
}

TEST(granular_no_nan)
{
    GranularEngine engine;
    engine.prepareToPlay(44100.0f);

    juce::AudioBuffer<float> source(1, 4410);
    auto* data = source.getWritePointer(0);
    for (int i = 0; i < 4410; ++i)
        data[i] = std::sin(2.0f * 3.14159265f * 220.0f * i / 44100.0f);

    engine.setSourceBuffer(&source, 44100.0);

    float outL[2048] = {}, outR[2048] = {};
    GranularParams params;
    params.grainSize = 20.0f;
    params.grainDensity = 50.0f;
    params.grainPosition = 0.3f;
    params.positionJitter = 0.5f;
    params.pitchJitter = 0.3f;
    params.panJitter = 0.5f;
    params.mode = GranularMode::Scatter;
    engine.renderBlock(outL, outR, 2048, params);

    for (int i = 0; i < 2048; ++i)
    {
        EXPECT(!std::isnan(outL[i]));
        EXPECT(!std::isnan(outR[i]));
    }
    return true;
}

TEST(granular_all_modes)
{
    GranularEngine engine;
    engine.prepareToPlay(44100.0f);

    juce::AudioBuffer<float> source(1, 44100);
    auto* data = source.getWritePointer(0);
    for (int i = 0; i < 44100; ++i)
        data[i] = std::sin(2.0f * 3.14159265f * 440.0f * i / 44100.0f);

    engine.setSourceBuffer(&source, 44100.0);

    GranularMode modes[] = {
        GranularMode::Scrub, GranularMode::Scan,
        GranularMode::Freeze, GranularMode::Scatter
    };

    for (auto mode : modes)
    {
        engine.reset();
        float outL[1024] = {}, outR[1024] = {};
        GranularParams params;
        params.grainSize = 30.0f;
        params.grainDensity = 20.0f;
        params.grainPosition = 0.5f;
        params.scanSpeed = 0.5f;
        params.mode = mode;
        engine.renderBlock(outL, outR, 1024, params);

        for (int i = 0; i < 1024; ++i)
        {
            EXPECT(!std::isnan(outL[i]));
            EXPECT(!std::isnan(outR[i]));
        }
    }
    return true;
}
