#include "fx/Reverb.h"
#include <algorithm>

namespace scenememo {

ReverbEffect::ReverbEffect() = default;

void ReverbEffect::prepareToPlay (float sr, int /*blockSize*/)
{
    sampleRate = sr;
    reverb.setSampleRate (static_cast<double> (sr));
    reset();
}

void ReverbEffect::reset()
{
    reverb.reset();
}

void ReverbEffect::processBlock (float* left, float* right, int numSamples, const ReverbParams& params)
{
    if (params.mix < 0.001f)
        return;

    juceReverbParams.roomSize = std::clamp (params.size, 0.0f, 1.0f);
    juceReverbParams.damping = std::clamp (params.damping, 0.0f, 1.0f);
    juceReverbParams.wetLevel = params.mix;
    juceReverbParams.dryLevel = 1.0f - params.mix;
    juceReverbParams.width = 1.0f;
    juceReverbParams.freezeMode = params.infinite ? 1.0f : 0.0f;

    reverb.setParameters (juceReverbParams);
    reverb.processStereo (left, right, numSamples);
}

} // namespace scenememo
