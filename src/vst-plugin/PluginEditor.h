/**
 * @file PluginEditor.h
 * @brief VST3 Plugin Editor with OpenGL rendering for projectM visualization.
 *
 * This file is part of projectM-SDL frontend.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include "PluginProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_opengl/juce_opengl.h>
#include <memory>
#include <atomic>

/**
 * @brief VST3 Plugin Editor with OpenGL rendering for projectM visualization.
 *
 * This editor creates an OpenGL context and renders projectM visualizations
 * in real-time based on the audio received by the processor.
 */
class ProjectMEditor : public juce::AudioProcessorEditor,
                       private juce::OpenGLRenderer,
                       private juce::Timer
{
public:
    explicit ProjectMEditor(ProjectMProcessor& processor);
    ~ProjectMEditor() override;

    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    // OpenGLRenderer overrides
    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

    // Timer callback
    void timerCallback() override;

    // UI Components
    void createControls();
    void updatePresetLabel();

    // Button callbacks
    void onPrevPreset();
    void onNextPreset();
    void onRandomPreset();
    void onToggleLock();
    void onOpenSettings();

    ProjectMProcessor& _processor;
    ProjectMVSTWrapper& _projectMWrapper;

    juce::OpenGLContext _openGLContext;

    // Control panel
    std::unique_ptr<juce::TextButton> _prevButton;
    std::unique_ptr<juce::TextButton> _nextButton;
    std::unique_ptr<juce::TextButton> _randomButton;
    std::unique_ptr<juce::TextButton> _lockButton;
    std::unique_ptr<juce::TextButton> _settingsButton;
    std::unique_ptr<juce::Label> _presetLabel;
    std::unique_ptr<juce::Label> _fpsLabel;

    // State
    std::atomic<bool> _isInitialized{false};
    std::atomic<bool> _isLocked{false};
    std::atomic<bool> _openGLAvailable{false};
    int _lastWidth{0};
    int _lastHeight{0};

    // FPS tracking
    double _lastFrameTime{0.0};
    int _frameCount{0};
    float _currentFps{0.0f};

    // Audio buffer for visualization
    std::vector<float> _audioBuffer;

    // Control panel height
    static constexpr int CONTROL_PANEL_HEIGHT = 40;
    static constexpr int BUTTON_WIDTH = 60;
    static constexpr int BUTTON_MARGIN = 5;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProjectMEditor)
};
