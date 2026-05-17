cmake_minimum_required(VERSION 3.22)
project(PerKungFu VERSION 0.1.0)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# JUCE via FetchContent
# ============================================================

include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(JUCE)

# ============================================================
# Plugin (VST3 + AU)
# ============================================================

juce_add_plugin(PerKungFu
    PRODUCT_NAME                    "PerKung-fu"
    COMPANY_NAME                    "Sampleson"
    PLUGIN_MANUFACTURER_CODE        Smps
    PLUGIN_CODE                     Pkfu
    FORMATS                         AU VST3
    IS_SYNTH                        FALSE
    NEEDS_MIDI_INPUT                TRUE
    IS_MIDI_EFFECT                  FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS     FALSE
    COPY_PLUGIN_AFTER_BUILD         TRUE
)

juce_generate_juce_header(PerKungFu)

# ============================================================
# Sources
# ============================================================

target_sources(PerKungFu PRIVATE
    Source/PluginProcessor.h
    Source/PluginProcessor.cpp
    Source/PluginEditor.h
    Source/PluginEditor.cpp
    Source/ParameterIDs.h
    Source/DSP/DrumVoice.h
)

target_include_directories(PerKungFu PRIVATE
    Source
    Source/DSP
)

# ============================================================
# Definitions & linking
# ============================================================

target_compile_definitions(PerKungFu PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
)

target_link_libraries(PerKungFu PRIVATE
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
