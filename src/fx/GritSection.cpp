#include "fx/GritSection.h"
#include <cmath>
#include <algorithm>

namespace scenememo {

GritSection::GritSection() = default;

void GritSection::prepareToPlay (float sr)
{
    sampleRate = sr;
    reset();
}

void GritSection::reset()
{
    tapeFilterState = 0.0f;
    wowPhase = 0.0f;
    crushHoldL = 0.0f;
    crushHoldR = 0.0f;
    crushCounter = 0.0f;
}

float GritSection::nextRng()
{
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    return static_cast<float> (rngState) / static_cast<float> (0xFFFFFFFFu);
}

float GritSection::softClip (float x)
{
    if (x > 1.5f) return 1.0f;
    if (x < -1.5f) return -1.0f;
    return x - (x * x * x) / 6.75f;
}

float GritSection::tapeWaveshaper (float input, float drive, float bias)
{
    float x = input * (1.0f + drive * 4.0f);

    // Asymmetric saturation based on bias (even/odd harmonic blend)
    float even = x * x * 0.5f;                    // even harmonics
    float odd = std::tanh (x);                      // odd harmonics
    float biased = odd * (1.0f - bias) + (odd + even * 0.3f) * bias;

    return softClip (biased);
}

void GritSection::processBlock (float* left, float* right, int numSamples, const GritParams& params)
{
    // Compute Decade macro overrides
    // decade: 0 = 70s (lots of character), 1 = modern (clean)
    float decadeInfluence = 1.0f - params.decade;
    float effectiveTapeDrive = params.tapeDrive + decadeInfluence * 0.4f;
    float effectiveVinylNoise = params.vinylNoise + decadeInfluence * 0.15f;
    float effectiveWowFlutter = params.wowFlutter + decadeInfluence * 0.3f;
    float highCutFromDecade = 20000.0f - decadeInfluence * 12000.0f; // roll off highs for vintage
    float stereoNarrow = 1.0f - decadeInfluence * 0.3f; // narrower stereo for vintage

    effectiveTapeDrive = std::clamp (effectiveTapeDrive, 0.0f, 1.0f);
    effectiveVinylNoise = std::clamp (effectiveVinylNoise, 0.0f, 1.0f);
    effectiveWowFlutter = std::clamp (effectiveWowFlutter, 0.0f, 1.0f);

    for (int i = 0; i < numSamples; ++i)
    {
        float L = left[i];
        float R = right[i];

        // --- Tape Saturation ---
        if (effectiveTapeDrive > 0.001f)
        {
            L = tapeWaveshaper (L, effectiveTapeDrive, params.tapeBias);
            R = tapeWaveshaper (R, effectiveTapeDrive, params.tapeBias);

            // Tape speed affects high frequency roll-off
            float cutoffs[] = { 8000.0f, 15000.0f, 20000.0f };
            float tapeCutoff = cutoffs[std::clamp (params.tapeSpeed, 0, 2)];
            float rc = 1.0f / (6.28318530718f * tapeCutoff);
            float alpha = (1.0f / sampleRate) / (rc + 1.0f / sampleRate);
            tapeFilterState += alpha * ((L + R) * 0.5f - tapeFilterState);
        }

        // --- Vinyl Character ---
        if (effectiveVinylNoise > 0.001f)
        {
            // Surface noise
            float noise = (nextRng() * 2.0f - 1.0f) * effectiveVinylNoise * 0.02f;
            L += noise;
            R += noise;

            // Crackle (random impulses)
            if (params.vinylCrackle > 0.001f && nextRng() < params.vinylCrackle * 0.001f)
            {
                float crackle = (nextRng() * 2.0f - 1.0f) * 0.3f;
                L += crackle;
                R += crackle;
            }
        }

        // --- Wow & Flutter ---
        if (effectiveWowFlutter > 0.001f)
        {
            float wowValue = std::sin (wowPhase * 6.28318530718f) * effectiveWowFlutter * 0.003f;
            wowPhase += params.wowRate / sampleRate;
            if (wowPhase >= 1.0f) wowPhase -= 1.0f;

            // Apply as pitch modulation (approximated as gain modulation for simplicity)
            L *= (1.0f + wowValue);
            R *= (1.0f + wowValue);
        }

        // --- Bit Crusher ---
        if (params.bitDepth < 23.5f || params.sampleRateReduce < 40000.0f)
        {
            // Sample rate reduction
            float crushStep = sampleRate / std::max (params.sampleRateReduce, 100.0f);
            crushCounter += 1.0f;
            if (crushCounter >= crushStep)
            {
                crushHoldL = L;
                crushHoldR = R;
                crushCounter -= crushStep;
            }
            L = crushHoldL;
            R = crushHoldR;

            // Bit depth reduction
            float levels = std::pow (2.0f, params.bitDepth);
            L = std::round (L * levels) / levels;
            R = std::round (R * levels) / levels;
        }

        // --- Decade high-cut filter ---
        if (highCutFromDecade < 19000.0f)
        {
            float rc = 1.0f / (6.28318530718f * highCutFromDecade);
            float alpha = (1.0f / sampleRate) / (rc + 1.0f / sampleRate);
            L = L * alpha + left[i] * (1.0f - alpha); // simplified LP
            R = R * alpha + right[i] * (1.0f - alpha);
        }

        // --- Decade stereo narrowing ---
        if (stereoNarrow < 0.99f)
        {
            float mid = (L + R) * 0.5f;
            float side = (L - R) * 0.5f;
            side *= stereoNarrow;
            L = mid + side;
            R = mid - side;
        }

        // --- Analog noise floor from Decade ---
        if (decadeInfluence > 0.01f)
        {
            float noiseFloor = (nextRng() * 2.0f - 1.0f) * decadeInfluence * 0.003f;
            L += noiseFloor;
            R += noiseFloor;
        }

        left[i] = L;
        right[i] = R;
    }
}

} // namespace scenememo
