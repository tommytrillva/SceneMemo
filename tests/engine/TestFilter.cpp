#include "../TestFramework.h"
#include "engine/MultiModeFilter.h"

using namespace scenememo;

TEST(filter_lowpass_attenuates_high)
{
    MultiModeFilter filt;
    filt.prepareToPlay(44100.0f);

    float sum = 0.0f;
    for (int i = 0; i < 4410; ++i)
    {
        float input = std::sin(2.0f * 3.14159265f * 10000.0f * i / 44100.0f);
        float output = filt.processSample(input, FilterType::LP24, 1000.0f, 0.0f, 0.0f, 0.0f, 440.0f);
        sum += output * output;
    }
    float rms = std::sqrt(sum / 4410.0f);
    EXPECT(rms < 0.1f);
    return true;
}

TEST(filter_lowpass_passes_low)
{
    MultiModeFilter filt;
    filt.prepareToPlay(44100.0f);

    float sum = 0.0f;
    for (int i = 0; i < 4410; ++i)
    {
        float input = std::sin(2.0f * 3.14159265f * 100.0f * i / 44100.0f);
        float output = filt.processSample(input, FilterType::LP24, 5000.0f, 0.0f, 0.0f, 0.0f, 440.0f);
        sum += output * output;
    }
    float rms = std::sqrt(sum / 4410.0f);
    EXPECT(rms > 0.5f);
    return true;
}

TEST(filter_no_nan_all_types)
{
    MultiModeFilter filt;
    filt.prepareToPlay(44100.0f);

    FilterType types[] = {
        FilterType::LP12, FilterType::LP24, FilterType::LP36,
        FilterType::HP12, FilterType::HP24,
        FilterType::BP, FilterType::Notch,
        FilterType::Comb, FilterType::Formant
    };

    for (auto type : types)
    {
        filt.reset();
        for (int i = 0; i < 1000; ++i)
        {
            float input = std::sin(2.0f * 3.14159265f * 440.0f * i / 44100.0f);
            float output = filt.processSample(input, type, 2000.0f, 0.5f, 0.3f, 0.0f, 440.0f);
            EXPECT(!std::isnan(output));
            EXPECT(!std::isinf(output));
        }
    }
    return true;
}

TEST(filter_resonance_boosts)
{
    MultiModeFilter filt;
    filt.prepareToPlay(44100.0f);

    float maxOutput = 0.0f;
    for (int i = 0; i < 4410; ++i)
    {
        float input = std::sin(2.0f * 3.14159265f * 1000.0f * i / 44100.0f) * 0.5f;
        float output = filt.processSample(input, FilterType::LP24, 1000.0f, 0.9f, 0.0f, 0.0f, 440.0f);
        maxOutput = std::max(maxOutput, std::abs(output));
    }
    EXPECT(maxOutput > 0.5f);
    return true;
}
