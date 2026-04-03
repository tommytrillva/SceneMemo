#include "../TestFramework.h"
#include "engine/Envelope.h"

using namespace scenememo;

TEST(envelope_starts_idle)
{
    AHDSREnvelope env;
    env.setSampleRate(44100.0f);
    EXPECT(!env.isActive());
    EXPECT(env.getState() == AHDSREnvelope::State::Idle);
    EXPECT_NEAR(env.processSample(), 0.0f, 0.0001f);
    return true;
}

TEST(envelope_attack_reaches_one)
{
    AHDSREnvelope env;
    env.setSampleRate(44100.0f);
    AHDSREnvelope::Parameters p;
    p.attack = 0.01f;
    p.hold = 0.0f;
    p.decay = 0.1f;
    p.sustain = 0.5f;
    p.release = 0.1f;
    env.setParameters(p);
    env.noteOn();

    float maxVal = 0.0f;
    for (int i = 0; i < 1000; ++i)
        maxVal = std::max(maxVal, env.processSample());

    EXPECT_NEAR(maxVal, 1.0f, 0.01f);
    return true;
}

TEST(envelope_sustain_level)
{
    AHDSREnvelope env;
    env.setSampleRate(44100.0f);
    AHDSREnvelope::Parameters p;
    p.attack = 0.001f;
    p.hold = 0.0f;
    p.decay = 0.05f;
    p.sustain = 0.6f;
    p.release = 0.1f;
    env.setParameters(p);
    env.noteOn();

    for (int i = 0; i < 10000; ++i)
        env.processSample();

    EXPECT(env.getState() == AHDSREnvelope::State::Sustain);
    EXPECT_NEAR(env.processSample(), 0.6f, 0.01f);
    return true;
}

TEST(envelope_release_to_idle)
{
    AHDSREnvelope env;
    env.setSampleRate(44100.0f);
    AHDSREnvelope::Parameters p;
    p.attack = 0.001f;
    p.hold = 0.0f;
    p.decay = 0.01f;
    p.sustain = 0.5f;
    p.release = 0.01f;
    env.setParameters(p);
    env.noteOn();

    for (int i = 0; i < 5000; ++i)
        env.processSample();

    env.noteOff();
    for (int i = 0; i < 5000; ++i)
        env.processSample();

    EXPECT(!env.isActive());
    return true;
}

TEST(envelope_no_nan)
{
    AHDSREnvelope env;
    env.setSampleRate(44100.0f);
    AHDSREnvelope::Parameters p;
    p.attack = 0.001f;
    p.hold = 0.1f;
    p.decay = 0.5f;
    p.sustain = 0.7f;
    p.release = 1.0f;
    p.attackCurve = 0.5f;
    p.decayCurve = -0.3f;
    p.releaseCurve = 0.8f;
    env.setParameters(p);
    env.noteOn();

    for (int i = 0; i < 44100; ++i)
    {
        float val = env.processSample();
        EXPECT(!std::isnan(val));
        EXPECT(!std::isinf(val));
        if (i == 22050) env.noteOff();
    }
    return true;
}
