#pragma once

#include <cstdint>

namespace scenememo {

// Audio engine
constexpr int kMaxVoices = 8;
constexpr int kDefaultWavetableSize = 2048;
constexpr int kMaxMipLevels = 10;
constexpr float kMinFrequency = 20.0f;
constexpr float kMaxFrequency = 20000.0f;
constexpr float kParamSmoothingSeconds = 0.02f; // 20ms default ramp

// Envelope defaults
constexpr float kDefaultAttack  = 0.01f;
constexpr float kDefaultHold    = 0.0f;
constexpr float kDefaultDecay   = 0.3f;
constexpr float kDefaultSustain = 0.7f;
constexpr float kDefaultRelease = 0.3f;

// Envelope range limits
constexpr float kMinEnvTime = 0.001f;
constexpr float kMaxAttack  = 5.0f;
constexpr float kMaxHold    = 2.0f;
constexpr float kMaxDecay   = 5.0f;
constexpr float kMaxRelease = 10.0f;

// Parameter IDs
namespace param {
    constexpr const char* kBypass     = "bypass";
    constexpr const char* kDryWet     = "drywet";
    constexpr const char* kMasterVol  = "master_vol";
    constexpr const char* kOscLevel   = "osc1_level";
    constexpr const char* kOscTune    = "osc1_tune";
    constexpr const char* kOscFine    = "osc1_fine";
    constexpr const char* kOscWaveform = "osc1_waveform";
    constexpr const char* kEnvAttack  = "env1_attack";
    constexpr const char* kEnvHold    = "env1_hold";
    constexpr const char* kEnvDecay   = "env1_decay";
    constexpr const char* kEnvSustain = "env1_sustain";
    constexpr const char* kEnvRelease = "env1_release";
}

// Waveform types
enum class Waveform : int {
    Sine = 0,
    Saw,
    Square,
    Triangle,
    NumWaveforms
};

} // namespace scenememo
