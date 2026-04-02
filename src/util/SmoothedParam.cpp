#include "util/SmoothedParam.h"

namespace scenememo {

void SmoothedParam::init (std::atomic<float>* paramValue, float smoothingTimeSeconds, float sampleRate)
{
    source = paramValue;
    smoothingTime = smoothingTimeSeconds;
    smoothed.reset (sampleRate, smoothingTime);

    if (source != nullptr)
        smoothed.setCurrentAndTargetValue (source->load());
}

void SmoothedParam::prepare (float sampleRate)
{
    smoothed.reset (sampleRate, smoothingTime);
}

void SmoothedParam::updateTarget()
{
    if (source != nullptr)
        smoothed.setTargetValue (source->load());
}

float SmoothedParam::getNextValue()
{
    return smoothed.getNextValue();
}

float SmoothedParam::getCurrentValue() const
{
    return smoothed.getCurrentValue();
}

bool SmoothedParam::isSmoothing() const
{
    return smoothed.isSmoothing();
}

} // namespace scenememo
