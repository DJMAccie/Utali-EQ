#pragma once

#if __has_include(<juce_audio_processors/juce_audio_processors.h>)
#include <juce_audio_processors/juce_audio_processors.h>
#elif __has_include(<JuceHeader.h>)
#include <JuceHeader.h>
#endif

#include "PluginProcessor.h"
#include "UI/UtaliLookAndFeel.h"
#include <memory>

class UTALITEQAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit UTALITEQAudioProcessorEditor(UTALITEQAudioProcessor&);
    ~UTALITEQAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    UTALITEQAudioProcessor& proc;
    UtaliLookAndFeel utaliLookAndFeel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    // --- Controls & Attachments ---
    // Low Band
    juce::Slider lowBoostSlider, lowAttenSlider, lowFreqSlider;
    juce::Label  lowBoostLabel, lowAttenLabel, lowFreqLabel;
    std::unique_ptr<SliderAttachment> lowBoostAtt, lowAttenAtt, lowFreqAtt;

    // Low-Mid Band
    juce::Slider lmfGainSlider, lmfFreqSlider, lmfQSlider;
    juce::Label  lmfGainLabel, lmfFreqLabel, lmfQLabel;
    std::unique_ptr<SliderAttachment> lmfGainAtt, lmfFreqAtt, lmfQAtt;

    // High-Mid Band
    juce::Slider hmfGainSlider, hmfFreqSlider, hmfQSlider;
    juce::Label  hmfGainLabel, hmfFreqLabel, hmfQLabel;
    std::unique_ptr<SliderAttachment> hmfGainAtt, hmfFreqAtt, hmfQAtt;

    // High Band
    juce::Slider highBoostSlider, highAttenSlider, highFreqSlider;
    juce::Label  highBoostLabel, highAttenLabel, highFreqLabel;
    std::unique_ptr<SliderAttachment> highBoostAtt, highAttenAtt, highFreqAtt;

    // Master / Character
    juce::Slider hpfSlider, driveSlider, outputSlider;
    juce::Label  hpfLabel, driveLabel, outputLabel;
    std::unique_ptr<SliderAttachment> hpfAtt, driveAtt, outputAtt;

    juce::ComboBox vibeBox;
    juce::Label    vibeLabel;
    std::unique_ptr<ComboBoxAttachment> vibeAtt;

    juce::Image backgroundImage;

    void setupRotarySlider(juce::Slider& slider, juce::Label& label,
                           const juce::String& text, const char* paramId,
                           std::unique_ptr<SliderAttachment>& attachment);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UTALITEQAudioProcessorEditor)
};
