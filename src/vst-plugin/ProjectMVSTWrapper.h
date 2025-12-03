/**
 * @file ProjectMVSTWrapper.h
 * @brief Wrapper class for projectM visualization library in VST context.
 *
 * This file is part of projectM-SDL frontend.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include <projectM-4/projectM.h>
#include <projectM-4/playlist.h>
#include <juce_core/juce_core.h>
#include <atomic>
#include <mutex>
#include <string>
#include <vector>

/**
 * @brief Wrapper class for projectM visualization library.
 *
 * This class encapsulates all projectM functionality for use within
 * the VST plugin context, handling initialization, rendering, and
 * preset management.
 */
class ProjectMVSTWrapper
{
public:
    ProjectMVSTWrapper();
    ~ProjectMVSTWrapper();

    /**
     * @brief Initializes projectM with the given window dimensions.
     * @param width Window width in pixels.
     * @param height Window height in pixels.
     * @return true if initialization succeeded.
     */
    bool initialize(int width, int height);

    /**
     * @brief Shuts down projectM and releases resources.
     */
    void shutdown();

    /**
     * @brief Checks if projectM is initialized.
     * @return true if initialized.
     */
    bool isInitialized() const { return _isInitialized; }

    /**
     * @brief Updates the window size.
     * @param width New width in pixels.
     * @param height New height in pixels.
     */
    void setWindowSize(int width, int height);

    /**
     * @brief Renders a single projectM frame.
     */
    void renderFrame();

    /**
     * @brief Adds audio samples for visualization.
     * @param samples Pointer to audio sample data.
     * @param count Number of samples.
     */
    void addAudioSamples(const float* samples, unsigned int count);

    // Preset management
    
    /**
     * @brief Gets the current preset name.
     * @return The name of the currently playing preset.
     */
    juce::String getCurrentPresetName() const;

    /**
     * @brief Gets the current preset index.
     * @return The index of the current preset in the playlist.
     */
    int getCurrentPresetIndex() const;

    /**
     * @brief Sets the preset to play by index.
     * @param index Preset index.
     */
    void setPresetIndex(int index);

    /**
     * @brief Moves to the next preset.
     */
    void nextPreset();

    /**
     * @brief Moves to the previous preset.
     */
    void previousPreset();

    /**
     * @brief Selects a random preset.
     */
    void randomPreset();

    /**
     * @brief Toggles the preset lock state.
     */
    void togglePresetLock();

    /**
     * @brief Checks if preset is locked.
     * @return true if preset is locked.
     */
    bool isPresetLocked() const;

    /**
     * @brief Sets the preset lock state.
     * @param locked true to lock, false to unlock.
     */
    void setPresetLocked(bool locked);

    /**
     * @brief Gets the number of presets in the playlist.
     * @return Preset count.
     */
    int getPresetCount() const;

    // Configuration

    /**
     * @brief Gets the preset path.
     * @return The current preset directory path.
     */
    juce::String getPresetPath() const;

    /**
     * @brief Sets the preset path and reloads presets.
     * @param path Path to preset directory.
     */
    void setPresetPath(const juce::String& path);

    /**
     * @brief Gets the texture path.
     * @return The current texture directory path.
     */
    juce::String getTexturePath() const;

    /**
     * @brief Sets the texture path.
     * @param path Path to texture directory.
     */
    void setTexturePath(const juce::String& path);

    /**
     * @brief Gets the beat sensitivity.
     * @return Current beat sensitivity value.
     */
    float getBeatSensitivity() const;

    /**
     * @brief Sets the beat sensitivity.
     * @param sensitivity Beat sensitivity value.
     */
    void setBeatSensitivity(float sensitivity);

    /**
     * @brief Gets the target FPS.
     * @return Target frames per second.
     */
    int getTargetFPS() const;

    /**
     * @brief Sets the target FPS.
     * @param fps Target frames per second.
     */
    void setTargetFPS(int fps);

private:
    /**
     * @brief Loads presets from the configured path.
     */
    void loadPresets();

    /**
     * @brief Static callback for preset switch events.
     */
    static void presetSwitchedCallback(bool isHardCut, unsigned int index, void* context);

    projectm_handle _projectM{nullptr};
    projectm_playlist_handle _playlist{nullptr};

    std::atomic<bool> _isInitialized{false};
    mutable std::mutex _mutex;

    juce::String _presetPath;
    juce::String _texturePath;
    juce::String _currentPresetName;

    float _beatSensitivity{1.0f};
    int _targetFPS{60};
    int _windowWidth{800};
    int _windowHeight{600};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProjectMVSTWrapper)
};
