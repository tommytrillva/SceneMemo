#pragma once

namespace scenememo {

struct MotionFXParams
{
    // Tremolo
    float tremoloRate = 4.0f;   // Hz (tempo-synced index or free Hz)
    float tremoloDepth = 0.0f;  // 0-1
    int tremoloShape = 0;       // 0=sine, 1=triangle, 2=square

    // Phaser
    float phaserRate = 0.5f;
    float phaserDepth = 0.0f;
    float phaserFeedback = 0.0f;  // -1 to +1
    int phaserStages = 4;         // 2,4,6,8,12

    // Chorus
    float chorusRate = 1.0f;
    float chorusDepth = 0.0f;
    int chorusVoices = 2;         // 2,4,6

    // Filter sweep
    float filterSweepRate = 1.0f;
    float filterSweepDepth = 0.0f;
    int filterSweepMode = 0;      // 0=LP, 1=HP, 2=BP
};

class MotionFX
{
public:
    MotionFX();

    void prepareToPlay (float sampleRate);
    void reset();

    void processBlock (float* left, float* right, int numSamples,
                       const MotionFXParams& params, float bpm);

private:
    float sampleRate = 44100.0f;

    // Tremolo LFO
    float tremoloPhase = 0.0f;

    // Phaser state (allpass filters)
    struct AllpassState { float y1 = 0.0f; };
    AllpassState phaserStatesL[6], phaserStatesR[6];
    float phaserLfoPhase = 0.0f;

    // Chorus delay line
    static constexpr int kChorusBufferSize = 4096;
    float chorusBufferL[kChorusBufferSize] = {};
    float chorusBufferR[kChorusBufferSize] = {};
    int chorusWritePos = 0;
    float chorusLfoPhase = 0.0f;

    // Filter sweep
    float sweepPhase = 0.0f;
    float sweepFilterStateL = 0.0f;
    float sweepFilterStateR = 0.0f;

    static float lfoShape (float phase, int shape);
};

} // namespace scenememo
