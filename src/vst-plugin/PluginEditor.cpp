/**
 * @file PluginEditor.cpp
 * @brief VST3 Plugin Editor implementation with OpenGL rendering.
 *
 * This file is part of projectM-SDL frontend.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "PluginEditor.h"

ProjectMEditor::ProjectMEditor(ProjectMProcessor& processor)
    : AudioProcessorEditor(&processor)
    , _processor(processor)
    , _projectMWrapper(processor.getProjectMWrapper())
{
    // Set initial size
    setSize(800, 600);
    setResizable(true, true);
    setResizeLimits(400, 300, 4096, 4096);

    // Create UI controls
    createControls();

    // Set up OpenGL context
    _openGLContext.setRenderer(this);
    _openGLContext.setContinuousRepainting(true);
    _openGLContext.setComponentPaintingEnabled(true);
    _openGLContext.attachTo(*this);

    // Start timer for FPS updates and UI refresh
    startTimerHz(30);
}

ProjectMEditor::~ProjectMEditor()
{
    stopTimer();
    _openGLContext.detach();
}

void ProjectMEditor::createControls()
{
    // Previous preset button
    _prevButton = std::make_unique<juce::TextButton>("<<");
    _prevButton->setTooltip("Previous Preset");
    _prevButton->onClick = [this] { onPrevPreset(); };
    addAndMakeVisible(_prevButton.get());

    // Next preset button
    _nextButton = std::make_unique<juce::TextButton>(">>");
    _nextButton->setTooltip("Next Preset");
    _nextButton->onClick = [this] { onNextPreset(); };
    addAndMakeVisible(_nextButton.get());

    // Random preset button
    _randomButton = std::make_unique<juce::TextButton>("Random");
    _randomButton->setTooltip("Random Preset");
    _randomButton->onClick = [this] { onRandomPreset(); };
    addAndMakeVisible(_randomButton.get());

    // Lock button
    _lockButton = std::make_unique<juce::TextButton>("Lock");
    _lockButton->setTooltip("Lock Current Preset");
    _lockButton->onClick = [this] { onToggleLock(); };
    addAndMakeVisible(_lockButton.get());

    // Settings button
    _settingsButton = std::make_unique<juce::TextButton>("Settings");
    _settingsButton->setTooltip("Open Settings");
    _settingsButton->onClick = [this] { onOpenSettings(); };
    addAndMakeVisible(_settingsButton.get());

    // Preset label
    _presetLabel = std::make_unique<juce::Label>("presetLabel", "Initializing...");
    _presetLabel->setJustificationType(juce::Justification::centredLeft);
    _presetLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(_presetLabel.get());

    // FPS label
    _fpsLabel = std::make_unique<juce::Label>("fpsLabel", "FPS: 0");
    _fpsLabel->setJustificationType(juce::Justification::centredRight);
    _fpsLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(_fpsLabel.get());
}

void ProjectMEditor::paint(juce::Graphics& g)
{
    // Fill background for control panel area only
    g.fillAll(juce::Colours::black);
    
    // Draw control panel background
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRect(0, getHeight() - CONTROL_PANEL_HEIGHT, getWidth(), CONTROL_PANEL_HEIGHT);
}

void ProjectMEditor::resized()
{
    const int width = getWidth();
    const int height = getHeight();
    const int controlY = height - CONTROL_PANEL_HEIGHT + (CONTROL_PANEL_HEIGHT - 25) / 2;

    int x = BUTTON_MARGIN;

    // Position buttons
    _prevButton->setBounds(x, controlY, BUTTON_WIDTH, 25);
    x += BUTTON_WIDTH + BUTTON_MARGIN;

    _nextButton->setBounds(x, controlY, BUTTON_WIDTH, 25);
    x += BUTTON_WIDTH + BUTTON_MARGIN;

    _randomButton->setBounds(x, controlY, BUTTON_WIDTH + 20, 25);
    x += BUTTON_WIDTH + 20 + BUTTON_MARGIN;

    _lockButton->setBounds(x, controlY, BUTTON_WIDTH, 25);
    x += BUTTON_WIDTH + BUTTON_MARGIN;

    _settingsButton->setBounds(x, controlY, BUTTON_WIDTH + 20, 25);
    x += BUTTON_WIDTH + 20 + BUTTON_MARGIN;

    // Position labels
    const int labelWidth = width - x - 80 - BUTTON_MARGIN * 2;
    _presetLabel->setBounds(x, controlY, labelWidth, 25);
    _fpsLabel->setBounds(width - 80 - BUTTON_MARGIN, controlY, 80, 25);

    // Notify projectM of size change
    const int renderHeight = height - CONTROL_PANEL_HEIGHT;
    if (_isInitialized && (width != _lastWidth || renderHeight != _lastHeight))
    {
        _projectMWrapper.setWindowSize(width, renderHeight);
        _lastWidth = width;
        _lastHeight = renderHeight;
    }
}

void ProjectMEditor::newOpenGLContextCreated()
{
    // Initialize projectM in the OpenGL context
    const int width = getWidth();
    const int height = getHeight() - CONTROL_PANEL_HEIGHT;
    
    if (_projectMWrapper.initialize(width, height))
    {
        _isInitialized = true;
        _lastWidth = width;
        _lastHeight = height;
        
        // Update preset label
        juce::MessageManager::callAsync([this] {
            updatePresetLabel();
        });
    }
}

void ProjectMEditor::renderOpenGL()
{
    if (!_isInitialized)
        return;

    // Get audio data from processor
    _processor.getAudioBuffer(_audioBuffer);
    
    // Pass audio to projectM
    if (!_audioBuffer.empty())
    {
        _projectMWrapper.addAudioSamples(_audioBuffer.data(), 
                                          static_cast<unsigned int>(_audioBuffer.size()));
    }

    // Set viewport for visualization (above control panel)
    const int width = getWidth();
    const int height = getHeight() - CONTROL_PANEL_HEIGHT;
    
    juce::gl::glViewport(0, CONTROL_PANEL_HEIGHT, width, height);

    // Render projectM frame
    _projectMWrapper.renderFrame();

    // Update FPS
    ++_frameCount;
    double currentTime = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    if (currentTime - _lastFrameTime >= 1.0)
    {
        _currentFps = static_cast<float>(_frameCount) / static_cast<float>(currentTime - _lastFrameTime);
        _frameCount = 0;
        _lastFrameTime = currentTime;
    }
}

void ProjectMEditor::openGLContextClosing()
{
    _isInitialized = false;
    _projectMWrapper.shutdown();
}

void ProjectMEditor::timerCallback()
{
    // Update FPS label
    if (_fpsLabel)
    {
        _fpsLabel->setText(juce::String::formatted("FPS: %.1f", _currentFps), 
                           juce::dontSendNotification);
    }

    // Update lock button state
    if (_lockButton)
    {
        bool locked = _projectMWrapper.isPresetLocked();
        _lockButton->setButtonText(locked ? "Unlock" : "Lock");
        _isLocked = locked;
    }
}

void ProjectMEditor::updatePresetLabel()
{
    if (_presetLabel && _isInitialized)
    {
        juce::String presetName = _projectMWrapper.getCurrentPresetName();
        if (presetName.isEmpty())
        {
            presetName = "No preset loaded";
        }
        _presetLabel->setText(presetName, juce::dontSendNotification);
    }
}

void ProjectMEditor::onPrevPreset()
{
    if (_isInitialized)
    {
        _projectMWrapper.previousPreset();
        juce::MessageManager::callAsync([this] {
            updatePresetLabel();
        });
    }
}

void ProjectMEditor::onNextPreset()
{
    if (_isInitialized)
    {
        _projectMWrapper.nextPreset();
        juce::MessageManager::callAsync([this] {
            updatePresetLabel();
        });
    }
}

void ProjectMEditor::onRandomPreset()
{
    if (_isInitialized)
    {
        _projectMWrapper.randomPreset();
        juce::MessageManager::callAsync([this] {
            updatePresetLabel();
        });
    }
}

void ProjectMEditor::onToggleLock()
{
    if (_isInitialized)
    {
        _projectMWrapper.togglePresetLock();
    }
}

void ProjectMEditor::onOpenSettings()
{
    // Show a dialog for configuring preset paths
    auto* dialogWindow = new juce::AlertWindow("projectM Settings", 
                                                "Configure visualization settings",
                                                juce::MessageBoxIconType::NoIcon);
    
    dialogWindow->addTextEditor("presetPath", 
                                _projectMWrapper.getPresetPath(), 
                                "Preset Path:");
    
    dialogWindow->addTextEditor("texturePath",
                                _projectMWrapper.getTexturePath(),
                                "Texture Path:");
    
    dialogWindow->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
    dialogWindow->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    dialogWindow->enterModalState(true, juce::ModalCallbackFunction::create(
        [this, dialogWindow](int result)
        {
            if (result == 1)
            {
                juce::String presetPath = dialogWindow->getTextEditorContents("presetPath");
                juce::String texturePath = dialogWindow->getTextEditorContents("texturePath");
                
                if (presetPath.isNotEmpty())
                {
                    _projectMWrapper.setPresetPath(presetPath);
                }
                if (texturePath.isNotEmpty())
                {
                    _projectMWrapper.setTexturePath(texturePath);
                }
                
                updatePresetLabel();
            }
            delete dialogWindow;
        }
    ), true);
}
