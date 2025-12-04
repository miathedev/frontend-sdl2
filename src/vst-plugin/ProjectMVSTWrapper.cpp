/**
 * @file ProjectMVSTWrapper.cpp
 * @brief Wrapper class implementation for projectM visualization library.
 *
 * This file is part of projectM-SDL frontend.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "ProjectMVSTWrapper.h"
#include <juce_opengl/juce_opengl.h>

ProjectMVSTWrapper::ProjectMVSTWrapper()
{
    // Default paths - users should configure these via settings
#if JUCE_MAC
    _presetPath = juce::File::getSpecialLocation(juce::File::userHomeDirectory)
                      .getChildFile("Library/Application Support/projectM/presets")
                      .getFullPathName();
    _texturePath = juce::File::getSpecialLocation(juce::File::userHomeDirectory)
                       .getChildFile("Library/Application Support/projectM/textures")
                       .getFullPathName();
#elif JUCE_WINDOWS
    _presetPath = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                      .getChildFile("projectM/presets").getFullPathName();
    _texturePath = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                       .getChildFile("projectM/textures").getFullPathName();
#else
    _presetPath = juce::File::getSpecialLocation(juce::File::userHomeDirectory)
                      .getChildFile(".config/projectM/presets")
                      .getFullPathName();
    _texturePath = juce::File::getSpecialLocation(juce::File::userHomeDirectory)
                       .getChildFile(".config/projectM/textures")
                       .getFullPathName();
#endif
}

ProjectMVSTWrapper::~ProjectMVSTWrapper()
{
    shutdown();
}

bool ProjectMVSTWrapper::initialize(int width, int height)
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_isInitialized)
        return true;

    // Validate dimensions
    if (width <= 0 || height <= 0)
        return false;

    _windowWidth = width;
    _windowHeight = height;

    try
    {
        // Create projectM instance
        _projectM = projectm_create();
        if (!_projectM)
        {
            return false;
        }

        // Configure projectM
        projectm_set_window_size(_projectM, width, height);
        projectm_set_fps(_projectM, static_cast<uint32_t>(_targetFPS));
        projectm_set_mesh_size(_projectM, 48, 32);
        projectm_set_aspect_correction(_projectM, true);
        projectm_set_preset_locked(_projectM, false);
        projectm_set_preset_duration(_projectM, 30.0);
        projectm_set_soft_cut_duration(_projectM, 3.0);
        projectm_set_hard_cut_enabled(_projectM, true);
        projectm_set_hard_cut_duration(_projectM, 20.0);
        projectm_set_hard_cut_sensitivity(_projectM, 1.0f);
        projectm_set_beat_sensitivity(_projectM, _beatSensitivity);

        // Set texture paths if available
        juce::File textureDir(_texturePath);
        if (textureDir.isDirectory())
        {
            std::string texturePathUtf8 = _texturePath.toStdString();
            const char* texturePaths[] = { texturePathUtf8.c_str() };
            projectm_set_texture_search_paths(_projectM, texturePaths, 1);
        }

        // Create playlist
        _playlist = projectm_playlist_create(_projectM);
        if (!_playlist)
        {
            projectm_destroy(_projectM);
            _projectM = nullptr;
            return false;
        }

        projectm_playlist_set_shuffle(_playlist, true);

        // Set preset switch callback
        projectm_playlist_set_preset_switched_event_callback(
            _playlist, &ProjectMVSTWrapper::presetSwitchedCallback, this);

        // Load presets
        loadPresets();

        // Start with first preset if available
        if (projectm_playlist_size(_playlist) > 0)
        {
            projectm_playlist_set_position(_playlist, 0, true);
        }

        _isInitialized = true;
        return true;
    }
    catch (...)
    {
        // Cleanup on error
        if (_playlist)
        {
            projectm_playlist_destroy(_playlist);
            _playlist = nullptr;
        }
        if (_projectM)
        {
            projectm_destroy(_projectM);
            _projectM = nullptr;
        }
        return false;
    }
}

void ProjectMVSTWrapper::shutdown()
{
    std::lock_guard<std::mutex> lock(_mutex);

    try
    {
        if (_playlist)
        {
            projectm_playlist_destroy(_playlist);
            _playlist = nullptr;
        }

        if (_projectM)
        {
            projectm_destroy(_projectM);
            _projectM = nullptr;
        }
    }
    catch (...)
    {
        // Ensure pointers are nullified even on error
        _playlist = nullptr;
        _projectM = nullptr;
    }

    _isInitialized = false;
}

void ProjectMVSTWrapper::setWindowSize(int width, int height)
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_projectM && width > 0 && height > 0)
    {
        _windowWidth = width;
        _windowHeight = height;
        projectm_set_window_size(_projectM, width, height);
    }
}

void ProjectMVSTWrapper::renderFrame()
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (!_projectM || !_isInitialized)
        return;

    try
    {
        // Clear and render
        juce::gl::glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        juce::gl::glClear(juce::gl::GL_COLOR_BUFFER_BIT | juce::gl::GL_DEPTH_BUFFER_BIT);

        projectm_opengl_render_frame(_projectM);
    }
    catch (...)
    {
        // Handle rendering errors gracefully
    }
}

void ProjectMVSTWrapper::addAudioSamples(const float* samples, unsigned int count)
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_projectM && samples && count > 0)
    {
        projectm_pcm_add_float(_projectM, samples, count, PROJECTM_MONO);
    }
}

juce::String ProjectMVSTWrapper::getCurrentPresetName() const
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (!_playlist)
        return {};

    unsigned int index = projectm_playlist_get_position(_playlist);
    char* name = projectm_playlist_item(_playlist, index);
    if (name)
    {
        juce::String result(name);
        projectm_playlist_free_string(name);
        
        // Extract just the filename without path and extension
        juce::File presetFile(result);
        return presetFile.getFileNameWithoutExtension();
    }
    return {};
}

int ProjectMVSTWrapper::getCurrentPresetIndex() const
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (!_playlist)
        return -1;

    return static_cast<int>(projectm_playlist_get_position(_playlist));
}

void ProjectMVSTWrapper::setPresetIndex(int index)
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_playlist && index >= 0 && 
        static_cast<unsigned int>(index) < projectm_playlist_size(_playlist))
    {
        projectm_playlist_set_position(_playlist, static_cast<unsigned int>(index), true);
    }
}

void ProjectMVSTWrapper::nextPreset()
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_playlist)
    {
        projectm_playlist_play_next(_playlist, true);
    }
}

void ProjectMVSTWrapper::previousPreset()
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_playlist)
    {
        projectm_playlist_play_previous(_playlist, true);
    }
}

void ProjectMVSTWrapper::randomPreset()
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_playlist)
    {
        // Temporarily enable shuffle, play next, then restore
        bool wasShuffled = projectm_playlist_get_shuffle(_playlist);
        projectm_playlist_set_shuffle(_playlist, true);
        projectm_playlist_play_next(_playlist, true);
        projectm_playlist_set_shuffle(_playlist, wasShuffled);
    }
}

void ProjectMVSTWrapper::togglePresetLock()
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_projectM)
    {
        bool currentLock = projectm_get_preset_locked(_projectM);
        projectm_set_preset_locked(_projectM, !currentLock);
    }
}

bool ProjectMVSTWrapper::isPresetLocked() const
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_projectM)
    {
        return projectm_get_preset_locked(_projectM);
    }
    return false;
}

void ProjectMVSTWrapper::setPresetLocked(bool locked)
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_projectM)
    {
        projectm_set_preset_locked(_projectM, locked);
    }
}

int ProjectMVSTWrapper::getPresetCount() const
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_playlist)
    {
        return static_cast<int>(projectm_playlist_size(_playlist));
    }
    return 0;
}

juce::String ProjectMVSTWrapper::getPresetPath() const
{
    return _presetPath;
}

void ProjectMVSTWrapper::setPresetPath(const juce::String& path)
{
    std::lock_guard<std::mutex> lock(_mutex);

    _presetPath = path;

    if (_playlist && _isInitialized)
    {
        // Clear and reload presets
        projectm_playlist_clear(_playlist);
        loadPresets();
    }
}

juce::String ProjectMVSTWrapper::getTexturePath() const
{
    return _texturePath;
}

void ProjectMVSTWrapper::setTexturePath(const juce::String& path)
{
    std::lock_guard<std::mutex> lock(_mutex);

    _texturePath = path;

    if (_projectM)
    {
        juce::File textureDir(path);
        if (textureDir.isDirectory())
        {
            std::string texturePathStr = path.toStdString();
            const char* texturePaths[] = { texturePathStr.c_str() };
            projectm_set_texture_search_paths(_projectM, texturePaths, 1);
        }
    }
}

float ProjectMVSTWrapper::getBeatSensitivity() const
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_projectM)
    {
        return projectm_get_beat_sensitivity(_projectM);
    }
    return _beatSensitivity;
}

void ProjectMVSTWrapper::setBeatSensitivity(float sensitivity)
{
    std::lock_guard<std::mutex> lock(_mutex);

    _beatSensitivity = sensitivity;

    if (_projectM)
    {
        projectm_set_beat_sensitivity(_projectM, sensitivity);
    }
}

int ProjectMVSTWrapper::getTargetFPS() const
{
    return _targetFPS;
}

void ProjectMVSTWrapper::setTargetFPS(int fps)
{
    std::lock_guard<std::mutex> lock(_mutex);

    _targetFPS = fps;

    if (_projectM)
    {
        projectm_set_fps(_projectM, static_cast<uint32_t>(fps));
    }
}

void ProjectMVSTWrapper::loadPresets()
{
    // Note: This method must be called with _mutex already locked by the caller

    if (!_playlist)
        return;

    juce::File presetDir(_presetPath);
    if (presetDir.isDirectory())
    {
        std::string presetPathStr = _presetPath.toStdString();
        projectm_playlist_add_path(_playlist, presetPathStr.c_str(), true, false);
        projectm_playlist_sort(_playlist, 0, projectm_playlist_size(_playlist),
                               SORT_PREDICATE_FILENAME_ONLY, SORT_ORDER_ASCENDING);
    }
}

void ProjectMVSTWrapper::presetSwitchedCallback(bool /*isHardCut*/, 
                                                 unsigned int index, 
                                                 void* context)
{
    auto* wrapper = static_cast<ProjectMVSTWrapper*>(context);
    if (wrapper)
    {
        std::lock_guard<std::mutex> lock(wrapper->_mutex);
        if (wrapper->_playlist)
        {
            char* name = projectm_playlist_item(wrapper->_playlist, index);
            if (name)
            {
                wrapper->_currentPresetName = juce::String(name);
                projectm_playlist_free_string(name);
            }
        }
    }
}
