#include "field/FieldEngine.h"
#include <cmath>

namespace scenememo {

FieldEngine::FieldEngine() = default;
FieldEngine::~FieldEngine() = default;

void FieldEngine::prepareToPlay (float sr, int blockSize)
{
    sampleRate = sr;
    granular.prepareToPlay (sr);
    spectralFreeze.prepareToPlay (sr);
    recorder.prepareToPlay (sr, blockSize);
}

void FieldEngine::reset()
{
    granular.reset();
    spectralFreeze.reset();
}

void FieldEngine::loadAudioFile (const juce::File& file, double targetSampleRate)
{
    auto audio = importer.loadFile (file, targetSampleRate);
    if (audio == nullptr)
        return;

    importedAudio = std::move (audio);
    onAudioLoaded();

    // Run analysis (blocking for now; background thread in production)
    analyzing.store (true);
    analysisResult = analyzer.analyze (importedAudio->buffer, importedAudio->sampleRate);
    analyzing.store (false);
}

void FieldEngine::loadFromRecorder()
{
    if (recorder.getRecordedLength() == 0)
        return;

    auto audio = std::make_unique<ImportedAudio>();
    audio->numSamples = recorder.getRecordedLength();
    audio->numChannels = 1;
    audio->sampleRate = sampleRate;
    audio->fileName = "Moment Recording";

    audio->buffer.setSize (1, audio->numSamples);
    audio->buffer.copyFrom (0, 0, recorder.getRecordedBuffer(), 0, 0, audio->numSamples);

    importedAudio = std::move (audio);
    onAudioLoaded();

    analyzing.store (true);
    analysisResult = analyzer.analyze (importedAudio->buffer, importedAudio->sampleRate);
    analyzing.store (false);
}

void FieldEngine::onAudioLoaded()
{
    if (importedAudio != nullptr)
    {
        granular.setSourceBuffer (&importedAudio->buffer, importedAudio->sampleRate);
        spectralFreeze.reset();
    }
}

void FieldEngine::triggerFreeze (int fftSizeChoice)
{
    if (importedAudio == nullptr)
        return;

    // Freeze at current grain position (middle of buffer as default)
    int position = importedAudio->numSamples / 2;
    spectralFreeze.freeze (importedAudio->buffer, position, fftSizeChoice);
}

void FieldEngine::renderBlock (float* outputL, float* outputR, int numSamples,
                                const FieldEngineParams& params, int midiNote)
{
    if (! hasAudioLoaded())
        return;

    float level = params.level;
    if (level < 0.0001f)
        return;

    // Temp buffers for separate sources
    std::vector<float> tempL (numSamples, 0.0f);
    std::vector<float> tempR (numSamples, 0.0f);

    // Granular contribution
    float granularLevel = 1.0f - params.freezeBlend;
    if (granularLevel > 0.001f)
    {
        granular.renderBlock (tempL.data(), tempR.data(), numSamples, params.granular, midiNote);

        for (int i = 0; i < numSamples; ++i)
        {
            tempL[i] *= granularLevel;
            tempR[i] *= granularLevel;
        }
    }

    // Spectral freeze contribution
    if (params.freezeEnabled && spectralFreeze.isFrozen() && params.freezeBlend > 0.001f)
    {
        std::vector<float> freezeL (numSamples, 0.0f);
        std::vector<float> freezeR (numSamples, 0.0f);

        spectralFreeze.renderBlock (freezeL.data(), freezeR.data(), numSamples,
                                    params.spectral, params.freezePitchShift);

        for (int i = 0; i < numSamples; ++i)
        {
            tempL[i] += freezeL[i] * params.freezeBlend;
            tempR[i] += freezeR[i] * params.freezeBlend;
        }
    }

    // Mix to output with level
    for (int i = 0; i < numSamples; ++i)
    {
        outputL[i] += tempL[i] * level;
        outputR[i] += tempR[i] * level;
    }
}

} // namespace scenememo
