#include "fx/MotionFX.h"
#include <cmath>
#include <algorithm>

namespace scenememo {

MotionFX::MotionFX() = default;

void MotionFX::prepareToPlay (float sr)
{
    sampleRate = sr;
    reset();
}

void MotionFX::reset()
{
    tremoloPhase = 0.0f;
    phaserLfoPhase = 0.0f;
    chorusWritePos = 0;
    chorusLfoPhase = 0.0f;
    sweepPhase = 0.0f;
    sweepFilterStateL = 0.0f;
    sweepFilterStateR = 0.0f;

    for (auto& s : phaserStatesL) s.y1 = 0.0f;
    for (auto& s : phaserStatesR) s.y1 = 0.0f;
    for (auto& s : chorusBufferL) s = 0.0f;
    for (auto& s : chorusBufferR) s = 0.0f;
}

float MotionFX::lfoShape (float phase, int shape)
{
    switch (shape)
    {
        case 0: return std::sin (phase * 6.28318530718f);
        case 1: return (phase < 0.25f) ? phase * 4.0f : (phase < 0.75f) ? 2.0f - phase * 4.0f : phase * 4.0f - 4.0f;
        case 2: return phase < 0.5f ? 1.0f : -1.0f;
        default: return std::sin (phase * 6.28318530718f);
    }
}

void MotionFX::processBlock (float* left, float* right, int numSamples,
                              const MotionFXParams& params, float /*bpm*/)
{
    for (int i = 0; i < numSamples; ++i)
    {
        float L = left[i];
        float R = right[i];

        // --- Tremolo ---
        if (params.tremoloDepth > 0.001f)
        {
            float trem = lfoShape (tremoloPhase, params.tremoloShape);
            float tremGain = 1.0f - params.tremoloDepth * 0.5f * (1.0f - trem);
            L *= tremGain;
            R *= tremGain;
            tremoloPhase += params.tremoloRate / sampleRate;
            if (tremoloPhase >= 1.0f) tremoloPhase -= 1.0f;
        }

        // --- Phaser ---
        if (params.phaserDepth > 0.001f)
        {
            float lfo = std::sin (phaserLfoPhase * 6.28318530718f);
            float centerFreq = 1000.0f + lfo * params.phaserDepth * 900.0f;
            centerFreq = std::clamp (centerFreq, 100.0f, 10000.0f);

            float d = -std::cos (6.28318530718f * centerFreq / sampleRate);
            int stages = std::clamp (params.phaserStages / 2, 1, 6);

            float pL = L, pR = R;
            for (int s = 0; s < stages; ++s)
            {
                float newL = d * pL + phaserStatesL[s].y1;
                phaserStatesL[s].y1 = pL - d * newL;
                pL = newL;

                float newR = d * pR + phaserStatesR[s].y1;
                phaserStatesR[s].y1 = pR - d * newR;
                pR = newR;
            }

            float fb = params.phaserFeedback;
            L = L + pL * params.phaserDepth + pL * fb * 0.5f;
            R = R + pR * params.phaserDepth + pR * fb * 0.5f;

            phaserLfoPhase += params.phaserRate / sampleRate;
            if (phaserLfoPhase >= 1.0f) phaserLfoPhase -= 1.0f;
        }

        // --- Chorus ---
        if (params.chorusDepth > 0.001f)
        {
            chorusBufferL[chorusWritePos] = L;
            chorusBufferR[chorusWritePos] = R;

            float lfo = std::sin (chorusLfoPhase * 6.28318530718f);
            float delayMs = 7.0f + lfo * params.chorusDepth * 5.0f; // 2-12ms
            float delaySamples = delayMs * sampleRate / 1000.0f;

            int readPos = chorusWritePos - static_cast<int> (delaySamples);
            if (readPos < 0) readPos += kChorusBufferSize;

            float wetL = chorusBufferL[readPos % kChorusBufferSize];
            float wetR = chorusBufferR[readPos % kChorusBufferSize];

            L = L * 0.7f + wetL * 0.3f * params.chorusDepth;
            R = R * 0.7f + wetR * 0.3f * params.chorusDepth;

            chorusWritePos = (chorusWritePos + 1) % kChorusBufferSize;
            chorusLfoPhase += params.chorusRate / sampleRate;
            if (chorusLfoPhase >= 1.0f) chorusLfoPhase -= 1.0f;
        }

        // --- Filter Sweep ---
        if (params.filterSweepDepth > 0.001f)
        {
            float lfo = std::sin (sweepPhase * 6.28318530718f);
            float cutoff = 1000.0f + lfo * params.filterSweepDepth * 8000.0f;
            cutoff = std::clamp (cutoff, 100.0f, 18000.0f);

            float rc = 1.0f / (6.28318530718f * cutoff);
            float alpha = (1.0f / sampleRate) / (rc + 1.0f / sampleRate);

            sweepFilterStateL += alpha * (L - sweepFilterStateL);
            sweepFilterStateR += alpha * (R - sweepFilterStateR);

            if (params.filterSweepMode == 0) // LP
            {
                L = sweepFilterStateL;
                R = sweepFilterStateR;
            }
            else if (params.filterSweepMode == 1) // HP
            {
                L = L - sweepFilterStateL;
                R = R - sweepFilterStateR;
            }
            else // BP
            {
                L = (L - sweepFilterStateL) * 0.5f + sweepFilterStateL * 0.5f;
                R = (R - sweepFilterStateR) * 0.5f + sweepFilterStateR * 0.5f;
            }

            sweepPhase += params.filterSweepRate / sampleRate;
            if (sweepPhase >= 1.0f) sweepPhase -= 1.0f;
        }

        left[i] = L;
        right[i] = R;
    }
}

} // namespace scenememo
