cmake_minimum_required(VERSION 3.22)
project(Jamo VERSION 0.1.0 LANGUAGES CXX)

option(JAMO_BUILD_TESTS "Build Jamo unit tests" ON)
option(JAMO_FETCH_JUCE "Fetch JUCE when not available locally" ON)

if(JAMO_FETCH_JUCE)
    include(FetchContent)
    FetchContent_Declare(juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.10
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(juce)
else()
    find_package(JUCE CONFIG REQUIRED)
endif()

juce_add_plugin(Jamo
    COMPANY_NAME "Jamo Audio"
    BUNDLE_ID "audio.jamo.vocaltuning"
    PLUGIN_MANUFACTURER_CODE Jamo
    PLUGIN_CODE Jamo
    FORMATS VST3 Standalone AU
    PRODUCT_NAME "Jamo"
    DESCRIPTION "Real-time vocal pitch correction"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE)

juce_generate_juce_header(Jamo)

target_sources(Jamo PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/PitchDetector.cpp
    Source/PitchDetector.h
    Source/Scale.cpp
    Source/Scale.h)

target_compile_features(Jamo PUBLIC cxx_std_17)
target_compile_definitions(Jamo PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(Jamo PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_lto_flags
           juce::juce_recommended_warning_flags)

if(JAMO_BUILD_TESTS)
    juce_add_console_app(JamoTests PRODUCT_NAME "Jamo Tests")
    juce_generate_juce_header(JamoTests)
    target_sources(JamoTests PRIVATE
        Tests/main.cpp
        Source/PitchDetector.cpp
        Source/Scale.cpp)
    target_compile_features(JamoTests PRIVATE cxx_std_17)
    target_compile_definitions(JamoTests PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_link_libraries(JamoTests PRIVATE
        juce::juce_core juce::juce_audio_basics juce::juce_dsp
        juce::juce_recommended_config_flags)
endif()
