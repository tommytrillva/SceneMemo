#pragma once

namespace scenememo {

// Crossfader/morph between Scene Engine and Field Engine output.
class BlendMatrix
{
public:
    BlendMatrix() = default;

    // blend: 0 = 100% Scene, 1 = 100% Field
    void processBlock (const float* sceneL, const float* sceneR,
                       const float* fieldL, const float* fieldR,
                       float* outputL, float* outputR,
                       int numSamples, float blend);

private:
};

} // namespace scenememo
