#pragma once

#include "field/AudioImporter.h"
#include "field/SourceAnalyzer.h"
#include "field/GranularEngine.h"
#include "field/SpectralFreeze.h"
#include "field/MomentRecorder.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <memory>
#include <thread>
#include <atomic>

namespace scenememo {

// Parameters exposed for the Field Engine
struct FieldEngineParams
{
    // Granular parameters
    GranularParams granular;

    // Spectral freeze
    SpectralParams spectral;
    bool freezeEnabled = false;
    float freezePitchShift = 0.0f; // semitones

    // Blend: how much granular vs spectral freeze (0 = all granular, 1 = all freeze)
    float freezeBlend = 0.0f;

    // Overall level
    float level = 0.8f;
};

class FieldEngine
{
public:
    FieldEngine();
    ~FieldEngine();

    void prepareToPlay (float sampleRate, int blockSize);
    void reset();

    // Import audio from file (runs analysis on background thread)
    void loadAudioFile (const juce::File& file, double targetSampleRate);

    // Load audio from the Moment Recorder's buffer
    void loadFromRecorder();

    // Render Field Engine output into the stereo buffer (additive)
    void renderBlock (float* outputL, float* outputR, int numSamples,
                      const FieldEngineParams& params, int midiNote = -1);

    // Trigger spectral freeze at current grain position
    void triggerFreeze (int fftSizeChoice = 2);

    // Access sub-components
    AudioImporter& getImporter() { return importer; }
    MomentRecorder& getRecorder() { return recorder; }
    GranularEngine& getGranular() { return granular; }
    SpectralFreeze& getSpectralFreeze() { return spectralFreeze; }

    bool hasAudioLoaded() const { return importedAudio != nullptr; }
    bool isAnalyzing() const { return analyzing.load(); }
    const AnalysisResult* getAnalysis() const { return analysisResult.get(); }
    const ImportedAudio* getImportedAudio() const { return importedAudio.get(); }

private:
    AudioImporter importer;
    SourceAnalyzer analyzer;
    GranularEngine granular;
    SpectralFreeze spectralFreeze;
    MomentRecorder recorder;

    std::unique_ptr<ImportedAudio> importedAudio;
    std::unique_ptr<AnalysisResult> analysisResult;

    float sampleRate = 44100.0f;
    std::atomic<bool> analyzing { false };

    void onAudioLoaded();
};

} // namespace scenememo
