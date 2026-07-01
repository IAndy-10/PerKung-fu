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
    FORMATS                         AU VST3 Standalone
    IS_SYNTH                        FALSE
    NEEDS_MIDI_INPUT                TRUE
    IS_MIDI_EFFECT                  FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS     FALSE
    COPY_PLUGIN_AFTER_BUILD         TRUE
    MICROPHONE_PERMISSION_ENABLED   TRUE
    MICROPHONE_PERMISSION_TEXT      "PerKung-fu needs microphone access for contact mic input"
    PLIST_TO_MERGE                  "<plist><dict><key>NSCameraUsageDescription</key><string>PerKung-fu uses the camera to detect objects and map them to synthesis presets.</string></dict></plist>"
)

juce_generate_juce_header(PerKungFu)

# ============================================================
# WebUI — run npm run build before embedding index.html
# ============================================================

find_program(NPM_EXECUTABLE npm REQUIRED)

add_custom_command(
    OUTPUT  ${CMAKE_SOURCE_DIR}/WebUI/dist/index.html
    COMMAND ${NPM_EXECUTABLE} run build
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/WebUI
    DEPENDS ${CMAKE_SOURCE_DIR}/WebUI/src/App.svelte
            ${CMAKE_SOURCE_DIR}/WebUI/src/bridge/bridge.ts
            ${CMAKE_SOURCE_DIR}/WebUI/src/state/store.ts
            ${CMAKE_SOURCE_DIR}/WebUI/src/presets.ts
    COMMENT "Building WebUI (npm run build)..."
)

add_custom_target(WebUIBuild DEPENDS ${CMAKE_SOURCE_DIR}/WebUI/dist/index.html)

juce_add_binary_data(PerKungFuData SOURCES WebUI/dist/index.html)
add_dependencies(PerKungFuData WebUIBuild)

# ============================================================
# Sources
# ============================================================

target_sources(PerKungFu PRIVATE
    Source/PluginProcessor.h
    Source/PluginProcessor.cpp
    Source/PluginEditor.h
    Source/PluginEditor.cpp
    Source/ParameterIDs.h
    Source/WebviewBridge.h
    Source/DSP/DrumVoice.h
)

if(APPLE)
    # Compile PluginEditor.cpp as Objective-C++ so it can use WebKit/AVFoundation directly
    set_source_files_properties(Source/PluginEditor.cpp
        PROPERTIES COMPILE_FLAGS "-x objective-c++")
endif()

target_include_directories(PerKungFu PRIVATE
    Source
    Source/DSP
)

# ============================================================
# Definitions & linking
# ============================================================

target_compile_definitions(PerKungFu PRIVATE
    JUCE_WEB_BROWSER=1
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
)

if(APPLE)
    target_link_libraries(PerKungFu PRIVATE
        "-framework AVFoundation"
        "-framework WebKit"
    )
endif()

target_link_libraries(PerKungFu PRIVATE
    PerKungFuData
    juce::juce_audio_processors
    juce::juce_audio_devices
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
