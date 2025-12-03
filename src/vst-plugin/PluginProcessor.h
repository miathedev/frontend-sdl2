/**
 * @file PluginProcessor.h
 * @brief VST3 Audio Processor for projectM visualization plugin.
 *
 * This file is part of projectM-SDL frontend.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include "ProjectMVSTWrapper.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <mutex>
#include <vector>

/**
 * @brief VST3 Audio Processor for projectM visualization.
 *
 * This processor receives audio from the DAW and passes it to projectM
 * for visualization. The audio is passed through unmodified.
 */
class ProjectMProcessor : public juce::AudioProcessor
{
public:
    ProjectMProcessor();
    ~ProjectMProcessor() override;

    // AudioProcessor overrides
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    /**
     * @brief Returns the projectM wrapper instance.
     * @return Reference to the ProjectMVSTWrapper.
     */
    ProjectMVSTWrapper& getProjectMWrapper() { return _projectMWrapper; }

    /**
     * @brief Gets the current sample rate.
     * @return The sample rate in Hz.
     */
    double getCurrentSampleRate() const { return _sampleRate; }

    /**
     * @brief Gets the current audio buffer for visualization.
     *
     * Thread-safe method to retrieve the most recent audio samples
     * for the visualization renderer.
     *
     * @param[out] buffer Vector to receive the audio samples.
     */
    void getAudioBuffer(std::vector<float>& buffer);

    /**
     * @brief Parameter IDs for VST automation.
     */
    static constexpr const char* PARAM_BEAT_SENSITIVITY = "beatSensitivity";
    static constexpr const char* PARAM_PRESET_INDEX = "presetIndex";

private:
    ProjectMVSTWrapper _projectMWrapper;
    
    double _sampleRate{44100.0};
    int _samplesPerBlock{512};

    // Audio buffer for visualization (lock-free ring buffer approach)
    std::vector<float> _audioBuffer;
    std::mutex _audioBufferMutex;
    static constexpr size_t AUDIO_BUFFER_SIZE = 2048;

    // Current preset index
    std::atomic<int> _currentPresetIndex{0};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProjectMProcessor)
};
