add_executable(SceneMemoTests
    engine/TestEnvelope.cpp
    engine/TestWavetableOsc.cpp
    engine/TestFilter.cpp
    engine/TestLFO.cpp
    field/TestGranular.cpp
    TestMain.cpp
)

target_include_directories(SceneMemoTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(SceneMemoTests PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_formats
    juce::juce_audio_devices
    juce::juce_dsp
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
)

target_compile_definitions(SceneMemoTests PRIVATE
    JUCE_STANDALONE_APPLICATION=1
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

# Link the shared code library from the plugin
target_sources(SceneMemoTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/engine/WavetableOsc.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/Envelope.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/LFO.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/MultiModeFilter.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/VirtualAnalogOsc.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/NoiseOsc.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/SubOsc.cpp
    ${CMAKE_SOURCE_DIR}/src/field/GranularEngine.cpp
)

add_test(NAME SceneMemoTests COMMAND SceneMemoTests)
