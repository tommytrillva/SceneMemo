#include "../TestFramework.h"
#include "engine/WavetableOsc.h"

using namespace scenememo;

TEST(wavetable_sine_output)
{
    auto sine = WavetableFactory::createSine();
    WavetableOsc osc;
    osc.setWavetable(sine);
    osc.setFrequency(440.0f, 44100.0f);

    float maxVal = 0.0f, minVal = 0.0f;
    for (int i = 0; i < 200; ++i)
    {
        float val = osc.processSample();
        maxVal = std::max(maxVal, val);
        minVal = std::min(minVal, val);
    }

    EXPECT(maxVal > 0.9f);
    EXPECT(minVal < -0.9f);
    return true;
}

TEST(wavetable_no_nan)
{
    auto saw = WavetableFactory::createSaw();
    WavetableOsc osc;
    osc.setWavetable(saw);

    float freqs[] = { 20.0f, 440.0f, 2000.0f, 10000.0f, 18000.0f };
    for (float freq : freqs)
    {
        osc.reset();
        osc.setFrequency(freq, 44100.0f);
        for (int i = 0; i < 1000; ++i)
        {
            float val = osc.processSample();
            EXPECT(!std::isnan(val));
            EXPECT(!std::isinf(val));
        }
    }
    return true;
}

TEST(wavetable_mip_levels_exist)
{
    auto tri = WavetableFactory::createTriangle();
    EXPECT(tri->getNumMipLevels() >= 3);
    EXPECT(tri->getMipLevel(0).size == 2048);
    EXPECT(tri->getMipLevel(1).size == 1024);
    return true;
}

TEST(wavetable_square_dc_offset)
{
    auto square = WavetableFactory::createSquare();
    WavetableOsc osc;
    osc.setWavetable(square);
    osc.setFrequency(100.0f, 44100.0f);

    float sum = 0.0f;
    int n = 4410;
    for (int i = 0; i < n; ++i)
        sum += osc.processSample();

    float dc = sum / n;
    EXPECT_NEAR(dc, 0.0f, 0.05f);
    return true;
}

TEST(wavetable_factory_all_types)
{
    EXPECT(WavetableFactory::createSine() != nullptr);
    EXPECT(WavetableFactory::createSaw() != nullptr);
    EXPECT(WavetableFactory::createSquare() != nullptr);
    EXPECT(WavetableFactory::createTriangle() != nullptr);
    return true;
}
