cmake_minimum_required(VERSION 3.22)
project(BasicMicEffect VERSION 0.1.0)

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
# Standalone app
# ============================================================

juce_add_gui_app(BasicMicEffect
    PRODUCT_NAME "Basic Mic Effect"
    COMPANY_NAME "Sampleson"

    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "Basic Mic Effect needs microphone access for real-time audio processing."
)

juce_generate_juce_header(BasicMicEffect)

# ============================================================
# Sources
# ============================================================

target_sources(BasicMicEffect PRIVATE
    Source/Main.cpp
    Source/MainComponent.h
    Source/MainComponent.cpp
    Source/DSP/DrumVoice.h
)

target_include_directories(BasicMicEffect PRIVATE
    Source
    Source/DSP
)

# ============================================================
# Definitions & linking
# ============================================================

target_compile_definitions(BasicMicEffect PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
)

target_link_libraries(BasicMicEffect PRIVATE
    juce::juce_audio_utils
    juce::juce_gui_basics
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
