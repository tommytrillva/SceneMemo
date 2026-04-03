#pragma once

namespace scenememo {

struct MasterOutputParams
{
    float stereoWidth = 1.0f;   // 0-2 (0=mono, 1=normal, 2=hyper-wide)
    float eqLowGain = 0.0f;    // dB (-12 to +12)
    float eqMidGain = 0.0f;    // dB
    float eqHighGain = 0.0f;   // dB
    bool limiterEnabled = true;
    float limiterCeiling = 0.0f; // dB (typically 0 or -0.3)
    float dryWet = 1.0f;       // 0-1
    float outputLevel = 1.0f;  // linear (0 to ~2, representing -inf to +6dB)
};

class MasterOutput
{
public:
    MasterOutput() = default;

    void prepareToPlay (float sampleRate);
    void reset();

    void processBlock (float* left, float* right, int numSamples,
                       const MasterOutputParams& params);

private:
    float sampleRate = 44100.0f;

    // EQ filter states (simple shelving)
    float eqLowStateL = 0.0f, eqLowStateR = 0.0f;
    float eqHighStateL = 0.0f, eqHighStateR = 0.0f;

    static float softLimit (float x, float ceiling);
};

} // namespace scenememo
