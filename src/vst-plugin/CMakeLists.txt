# projectM VST3 Plugin CMake configuration
#
# This file is part of projectM-SDL frontend.
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

cmake_minimum_required(VERSION 3.22)

# JUCE is fetched via FetchContent
include(FetchContent)

# Fetch JUCE framework
# Using JUCE 8.0.4 for macOS 15 / Xcode 16 compatibility
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(JUCE)

# Define the VST3 plugin target
juce_add_plugin(projectM-VST
    COMPANY_NAME "projectM"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE PrjM
    PLUGIN_CODE PrVM
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "projectM Visualizer"
    VST3_CATEGORIES Analyzer Fx
    AU_MAIN_TYPE kAudioUnitType_Effect
    BUNDLE_ID "com.projectm.visualizer"
    MICROPHONE_PERMISSION_ENABLED FALSE
    HARDENED_RUNTIME_ENABLED TRUE
)

# Source files for the VST plugin
target_sources(projectM-VST
    PRIVATE
        PluginProcessor.cpp
        PluginProcessor.h
        PluginEditor.cpp
        PluginEditor.h
        ProjectMVSTWrapper.cpp
        ProjectMVSTWrapper.h
)

# Compile definitions
target_compile_definitions(projectM-VST
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

# Link required libraries
target_link_libraries(projectM-VST
    PRIVATE
        juce::juce_audio_utils
        juce::juce_opengl
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
        libprojectM::projectM
        libprojectM::playlist
)

# Set C++ standard
target_compile_features(projectM-VST PRIVATE cxx_std_17)
