#include "mixer/BlendMatrix.h"
#include <cmath>

namespace scenememo {

void BlendMatrix::processBlock (const float* sceneL, const float* sceneR,
                                 const float* fieldL, const float* fieldR,
                                 float* outputL, float* outputR,
                                 int numSamples, float blend)
{
    // Equal-power crossfade
    float sceneGain = std::cos (blend * 1.5707963f);
    float fieldGain = std::sin (blend * 1.5707963f);

    for (int i = 0; i < numSamples; ++i)
    {
        outputL[i] = sceneL[i] * sceneGain + fieldL[i] * fieldGain;
        outputR[i] = sceneR[i] * sceneGain + fieldR[i] * fieldGain;
    }
}

} // namespace scenememo
