cmake_minimum_required(VERSION 3.22)
project(SceneMemo VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- JUCE Discovery ---
# Priority: local submodule > FetchContent download
if(EXISTS "${CMAKE_SOURCE_DIR}/JUCE/CMakeLists.txt")
    message(STATUS "Using local JUCE submodule")
    add_subdirectory(JUCE)
else()
    message(STATUS "Fetching JUCE via FetchContent...")
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

# --- Plugin Target ---
juce_add_plugin(SceneMemo
    COMPANY_NAME "InvalidNRMLS"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    PLUGIN_MANUFACTURER_CODE Scnm
    PLUGIN_CODE Scm1
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "SceneMemo"
    COPY_PLUGIN_AFTER_BUILD TRUE
)

target_sources(SceneMemo
    PRIVATE
        src/PluginProcessor.cpp
        src/PluginEditor.cpp
        # Engine — oscillators
        src/engine/WavetableOsc.cpp
        src/engine/VirtualAnalogOsc.cpp
        src/engine/NoiseOsc.cpp
        src/engine/SubOsc.cpp
        src/engine/OscillatorSlot.cpp
        # Engine — core
        src/engine/Envelope.cpp
        src/engine/LFO.cpp
        src/engine/MultiModeFilter.cpp
        src/engine/ModMatrix.cpp
        src/engine/Voice.cpp
        src/engine/VoiceAllocator.cpp
        # Engine — parameters
        src/engine/ParameterLayout.cpp
        src/engine/ParamReader.cpp
        # Utilities
        src/util/SmoothedParam.cpp
        # Mixer
        src/mixer/MoodLanes.cpp
        src/mixer/BlendMatrix.cpp
        src/mixer/SceneMorph.cpp
        # FX chain
        src/fx/GritSection.cpp
        src/fx/MotionFX.cpp
        src/fx/SidechainModule.cpp
        src/fx/Reverb.cpp
        src/fx/Delay.cpp
        src/fx/MasterOutput.cpp
        # Field Engine
        src/field/AudioImporter.cpp
        src/field/SourceAnalyzer.cpp
        src/field/GranularEngine.cpp
        src/field/SpectralFreeze.cpp
        src/field/MomentRecorder.cpp
        src/field/FieldEngine.cpp
)

target_include_directories(SceneMemo
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_compile_definitions(SceneMemo
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_ALSA=1
)

target_link_libraries(SceneMemo
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_formats
        juce::juce_audio_devices
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# --- Tests (optional) ---
option(BUILD_TESTING "Build unit tests" OFF)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
