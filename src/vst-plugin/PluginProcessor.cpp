/**
 * @file PluginProcessor.cpp
 * @brief VST3 Audio Processor implementation for projectM visualization plugin.
 *
 * This file is part of projectM-SDL frontend.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

ProjectMProcessor::ProjectMProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    _audioBuffer.resize(AUDIO_BUFFER_SIZE, 0.0f);
}

ProjectMProcessor::~ProjectMProcessor()
{
}

const juce::String ProjectMProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ProjectMProcessor::acceptsMidi() const
{
    return false;
}

bool ProjectMProcessor::producesMidi() const
{
    return false;
}

bool ProjectMProcessor::isMidiEffect() const
{
    return false;
}

double ProjectMProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ProjectMProcessor::getNumPrograms()
{
    return 1;
}

int ProjectMProcessor::getCurrentProgram()
{
    return 0;
}

void ProjectMProcessor::setCurrentProgram(int /*index*/)
{
}

const juce::String ProjectMProcessor::getProgramName(int /*index*/)
{
    return {};
}

void ProjectMProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/)
{
}

void ProjectMProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    _sampleRate = sampleRate;
    _samplesPerBlock = samplesPerBlock;
}

void ProjectMProcessor::releaseResources()
{
    // Nothing to release
}

bool ProjectMProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // We support mono or stereo input/output
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Input and output layouts must match
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void ProjectMProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                     juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't have input
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Pass audio to projectM for visualization (mix down to mono)
    int numSamples = buffer.getNumSamples();
    
    if (numSamples > 0 && totalNumInputChannels > 0)
    {
        // Bounds check to prevent buffer overflow
        if (numSamples > static_cast<int>(AUDIO_BUFFER_SIZE))
            numSamples = static_cast<int>(AUDIO_BUFFER_SIZE);

        // Mix to mono in a temporary buffer (no locking required)
        std::vector<float> monoSamples(static_cast<size_t>(numSamples));
        const float* leftChannel = buffer.getReadPointer(0);
        const float* rightChannel = (totalNumInputChannels > 1) ? buffer.getReadPointer(1) : leftChannel;
        for (int i = 0; i < numSamples; ++i)
        {
            monoSamples[static_cast<size_t>(i)] = (leftChannel[i] + rightChannel[i]) * 0.5f;
        }

        // Now lock only for the buffer write operation
        std::lock_guard<std::mutex> lock(_audioBufferMutex);
        
        // Make room for new samples by shifting existing data
        const size_t samplesToKeep = AUDIO_BUFFER_SIZE - static_cast<size_t>(numSamples);
        if (samplesToKeep > 0 && static_cast<size_t>(numSamples) < AUDIO_BUFFER_SIZE)
        {
            std::memmove(_audioBuffer.data(), 
                        _audioBuffer.data() + numSamples,
                        samplesToKeep * sizeof(float));
        }
        
        // Copy new samples to the end of the buffer
        const size_t startIndex = (samplesToKeep > 0) ? samplesToKeep : 0;
        for (int i = 0; i < numSamples && (startIndex + static_cast<size_t>(i)) < AUDIO_BUFFER_SIZE; ++i)
        {
            _audioBuffer[startIndex + static_cast<size_t>(i)] = monoSamples[static_cast<size_t>(i)];
        }
    }

    // Audio passes through unmodified - this is a visualization-only plugin
}

juce::AudioProcessorEditor* ProjectMProcessor::createEditor()
{
    return new ProjectMEditor(*this);
}

bool ProjectMProcessor::hasEditor() const
{
    return true;
}

void ProjectMProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Store plugin state
    juce::ValueTree state("ProjectMState");
    
    // Store preset path if configured
    juce::String presetPath = _projectMWrapper.getPresetPath();
    if (presetPath.isNotEmpty())
    {
        state.setProperty("presetPath", presetPath, nullptr);
    }
    
    // Store current preset index
    state.setProperty("presetIndex", _currentPresetIndex.load(), nullptr);
    
    // Store beat sensitivity
    state.setProperty("beatSensitivity", _projectMWrapper.getBeatSensitivity(), nullptr);
    
    // Serialize to memory
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    if (xml)
    {
        copyXmlToBinary(*xml, destData);
    }
}

void ProjectMProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Restore plugin state
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml)
    {
        juce::ValueTree state = juce::ValueTree::fromXml(*xml);
        if (state.isValid() && state.getType().toString() == "ProjectMState")
        {
            // Restore preset path
            juce::String presetPath = state.getProperty("presetPath", "");
            if (presetPath.isNotEmpty())
            {
                _projectMWrapper.setPresetPath(presetPath);
            }
            
            // Restore preset index
            int presetIndex = state.getProperty("presetIndex", 0);
            _currentPresetIndex.store(presetIndex);
            _projectMWrapper.setPresetIndex(presetIndex);
            
            // Restore beat sensitivity
            float beatSensitivity = state.getProperty("beatSensitivity", 1.0f);
            _projectMWrapper.setBeatSensitivity(beatSensitivity);
        }
    }
}

void ProjectMProcessor::getAudioBuffer(std::vector<float>& buffer)
{
    std::lock_guard<std::mutex> lock(_audioBufferMutex);
    buffer = _audioBuffer;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ProjectMProcessor();
}
