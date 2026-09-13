#pragma once

#if __has_include(<juce_audio_processors/juce_audio_processors.h>)
#include <juce_audio_processors/juce_audio_processors.h>
#elif __has_include(<JuceHeader.h>)
#include <JuceHeader.h>
#endif
#include "Parameters.h"
#include "DSP/EqualizerEngine.h"
#include "DSP/SaturationProcessor.h"

class UTALITEQAudioProcessor : public juce::AudioProcessor
{
public:
    UTALITEQAudioProcessor();
    ~UTALITEQAudioProcessor() override = default;

    //==========================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==========================================================================
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;

    //==========================================================================
#ifdef JucePlugin_Name
    const juce::String getName() const override { return JucePlugin_Name; }
#else
    const juce::String getName() const override { return "UTALITEQ"; }
#endif
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==========================================================================
    int  getNumPrograms() override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    //==========================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return parameters; }

private:
    juce::AudioProcessorValueTreeState parameters;
    ParameterPointers paramPointers;

    EqualizerEngine equalizerEngine;
    SaturationProcessor saturationProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UTALITEQAudioProcessor)
};
