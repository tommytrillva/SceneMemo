# SceneMemo

**Cinematic Atmosphere Synth & Life-Sound Instrument**

SceneMemo is a tempo-synced cinematic atmosphere synthesizer that turns voice memos, field recordings, and daily life sounds into beat-ready, filmic soundscapes. It bridges the gap between scoring and modern production.

*"Every moment is an instrument."*

## Features

### Dual-Engine Architecture
- **Scene Engine** — Multi-oscillator cinematic synthesizer with 4 oscillator slots (Wavetable, Virtual Analog, Noise, Sub), dual SVF multi-mode filters, 4 tempo-syncable LFOs, 4 AHDSR envelopes with curve shaping, and a 16-slot modulation matrix
- **Field Engine** — Granular/spectral engine that ingests voice memos and field recordings, with HPSS source separation, 128-grain granular playback (5 modes), spectral freeze with additive resynthesis, and a built-in Moment Recorder

### Processing Chain
- **Grit Section** — Tape saturation, vinyl character, bit crusher, and the signature *Decade* knob (one macro that shifts character from modern to 70s analog)
- **Motion FX** — Tempo-synced tremolo, phaser, chorus, and filter sweep
- **Sidechain** — Built-in ducker with internal kick detection
- **Space** — Reverb (with infinite mode) and tempo-synced delay with feedback filtering
- **Master Output** — Stereo width, 3-band EQ, soft limiter

### Mixing & Morphing
- **Mood Lanes** — 4-lane parallel mixer (Low Warmth, Mid Harmonic, High Air, Rhythm Texture)
- **Blend Matrix** — Equal-power crossfade between Scene and Field Engine
- **Scene Morph** — A/B snapshot system with 4 morph curve shapes and per-parameter locking

### Sequencers
- **Texture Micro-Sequencer** — 16-step sequencer for texture events (reverse swell, filter sweep, spectral freeze, grain burst, etc.)
- **Foley Rhythm Designer** — 8-slot sample loader with 16-step rhythm sequencer

### Preset System
- **264 factory presets** across 12 Tonal Palette mood categories
- JSON-based `.smemo` format with rich metadata
- A/B comparison, favorites, category browsing

### Tonal Palette Categories
Golden Hour · Night Drive · Rooftop Rain · City Fog · Desert Heat · Neon Alley · Ocean Floor · Midnight Studio · First Light · Street Level · Memory Lane · The Cosmos

## Building

### Requirements
- CMake 3.22+
- C++20 compiler (GCC 13+, Clang 15+, MSVC 2022+)
- JUCE 8 (fetched automatically via CMake FetchContent)

### Linux Dependencies
```bash
sudo apt-get install libasound2-dev libxrandr-dev libxinerama-dev \
    libxcursor-dev libgl-dev libfreetype-dev
```

### Build Commands
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j$(nproc)
```

### Build Targets
- **Standalone**: `build/SceneMemo_artefacts/Release/Standalone/SceneMemo`
- **VST3**: `build/SceneMemo_artefacts/Release/VST3/SceneMemo.vst3/`
- **AU**: (macOS only)

### Running Tests
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release
cd build && ctest
```

## Architecture

```
src/
├── PluginProcessor.h/cpp      Main audio processor
├── PluginEditor.h/cpp         GUI (tabbed interface)
├── engine/                    Scene Engine (synthesis)
│   ├── WavetableOsc            Mip-mapped wavetable oscillator
│   ├── VirtualAnalogOsc        PolyBLEP anti-aliased VA oscillator
│   ├── NoiseOsc                White/pink/brown/air noise
│   ├── SubOsc                  Sub oscillator (-1/-2 octave)
│   ├── OscillatorSlot          Variant-based osc type switching + unison
│   ├── MultiModeFilter         SVF filter (9 types, cascadable)
│   ├── Envelope                AHDSR with curve shaping
│   ├── LFO                     8 shapes, tempo sync, humanize
│   ├── ModMatrix               16-slot modulation routing
│   ├── Voice                   Per-voice: 4 osc + 2 filt + 4 env + 4 LFO
│   ├── VoiceAllocator          8-voice polyphony, oldest-steal
│   ├── ParameterLayout         ~172 parameter factory
│   └── ParamReader             Bulk APVTS reader
├── field/                     Field Engine (granular/spectral)
│   ├── AudioImporter           Multi-format audio loading
│   ├── SourceAnalyzer          HPSS, pitch tracking, onset detection
│   ├── GranularEngine          128-grain engine, 5 playback modes
│   ├── SpectralFreeze          FFT capture + additive resynthesis
│   ├── MomentRecorder          Built-in audio recorder
│   └── FieldEngine             Coordinator wrapper
├── mixer/                     Mixing & morphing
│   ├── MoodLanes               4-lane parallel mixer
│   ├── BlendMatrix             Scene↔Field crossfade
│   └── SceneMorph              A/B scene snapshots
├── fx/                        Processing chain
│   ├── GritSection             Tape/vinyl/bitcrush/Decade
│   ├── MotionFX                Tremolo/phaser/chorus/filter sweep
│   ├── SidechainModule         Internal kick detection ducker
│   ├── Reverb                  JUCE reverb wrapper
│   ├── Delay                   Stereo delay with feedback filtering
│   └── MasterOutput            Width/EQ/limiter
├── sequencer/                 Sequencers
│   ├── MicroSequencer          16-step texture event sequencer
│   └── FoleyRhythm             8-slot foley rhythm designer
├── preset/                    Preset system
│   ├── PresetData              JSON serialization
│   └── PresetManager           Save/load/browse/A-B compare
└── util/                      Utilities
    ├── Constants.h              All constants and enums
    └── SmoothedParam            Parameter smoothing
```

## Plugin Formats
- **VST3** (Windows, macOS, Linux)
- **AU** (macOS)
- **Standalone** (all platforms)

## Technical Specs
- 32-bit float internal processing
- 8-voice polyphony (configurable)
- ~172 automatable parameters
- Sample-accurate MIDI processing
- Real-time safe audio thread (no allocations/locks in processBlock)
- Block-rate modulation matrix (32-sample sub-blocks)
- Host transport reading (BPM/PPQ) for tempo-synced LFOs

## License
Proprietary — © 2026 Tommy Trill / Invalid NRMLS. All rights reserved.
