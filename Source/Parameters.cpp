#include "Parameters.h"

juce::StringArray FrequencyTables::getLowFrequencyChoices()
{
    return { "30", "60", "100", "200" };
}

juce::StringArray FrequencyTables::getHighFrequencyChoices()
{
    return { "6000", "8000", "10000", "12000" };
}

juce::StringArray FrequencyTables::getVibeChoices()
{
    return { "Clean", "Transistor", "Tube", "Tape" };
}

void ParameterPointers::initialize(juce::AudioProcessorValueTreeState& apvts) noexcept
{
    hpfFreq  = apvts.getRawParameterValue(ParameterIDs::hpfFreq);
    lfFreq   = apvts.getRawParameterValue(ParameterIDs::lfFreq);
    lfBoost  = apvts.getRawParameterValue(ParameterIDs::lfBoost);
    lfAtten  = apvts.getRawParameterValue(ParameterIDs::lfAtten);
    lmfFreq  = apvts.getRawParameterValue(ParameterIDs::lmfFreq);
    lmfGain  = apvts.getRawParameterValue(ParameterIDs::lmfGain);
    lmfQ     = apvts.getRawParameterValue(ParameterIDs::lmfQ);
    hmfFreq  = apvts.getRawParameterValue(ParameterIDs::hmfFreq);
    hmfGain  = apvts.getRawParameterValue(ParameterIDs::hmfGain);
    hmfQ     = apvts.getRawParameterValue(ParameterIDs::hmfQ);
    hfFreq   = apvts.getRawParameterValue(ParameterIDs::hfFreq);
    hfBoost  = apvts.getRawParameterValue(ParameterIDs::hfBoost);
    hfAtten  = apvts.getRawParameterValue(ParameterIDs::hfAtten);
    vibeType = apvts.getRawParameterValue(ParameterIDs::vibeType);
    drive    = apvts.getRawParameterValue(ParameterIDs::drive);
    output   = apvts.getRawParameterValue(ParameterIDs::output);
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using APF = juce::AudioParameterFloat;
    using APC = juce::AudioParameterChoice;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<APF>(ParameterIDs::hpfFreq, "High‑Pass Freq",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 20.f));

    layout.add(std::make_unique<APC>(ParameterIDs::lfFreq, "Low Freq",
        FrequencyTables::getLowFrequencyChoices(), 1));

    layout.add(std::make_unique<APF>(ParameterIDs::lfBoost, "Low Boost",
        juce::NormalisableRange<float>(0.f, 15.f, 0.1f), 0.f));
    layout.add(std::make_unique<APF>(ParameterIDs::lfAtten, "Low Atten",
        juce::NormalisableRange<float>(0.f, 15.f, 0.1f), 0.f));

    layout.add(std::make_unique<APF>(ParameterIDs::lmfFreq, "LMF Freq",
        juce::NormalisableRange<float>(200.f, 2500.f, 1.f, 0.5f), 700.f));
    layout.add(std::make_unique<APF>(ParameterIDs::lmfGain, "LMF Gain",
        juce::NormalisableRange<float>(-15.f, 15.f, 0.1f), 0.f));
    layout.add(std::make_unique<APF>(ParameterIDs::lmfQ, "LMF Q",
        juce::NormalisableRange<float>(0.1f, 10.f, 0.01f, 0.5f), 0.71f));

    layout.add(std::make_unique<APF>(ParameterIDs::hmfFreq, "HMF Freq",
        juce::NormalisableRange<float>(1500.f, 7500.f, 1.f, 0.5f), 3000.f));
    layout.add(std::make_unique<APF>(ParameterIDs::hmfGain, "HMF Gain",
        juce::NormalisableRange<float>(-15.f, 15.f, 0.1f), 0.f));
    layout.add(std::make_unique<APF>(ParameterIDs::hmfQ, "HMF Q",
        juce::NormalisableRange<float>(0.1f, 10.f, 0.01f, 0.5f), 0.71f));

    layout.add(std::make_unique<APC>(ParameterIDs::hfFreq, "High Freq",
        FrequencyTables::getHighFrequencyChoices(), 2));
    layout.add(std::make_unique<APF>(ParameterIDs::hfBoost, "High Boost",
        juce::NormalisableRange<float>(0.f, 15.f, 0.1f), 0.f));
    layout.add(std::make_unique<APF>(ParameterIDs::hfAtten, "High Atten",
        juce::NormalisableRange<float>(0.f, 15.f, 0.1f), 0.f));

    layout.add(std::make_unique<APC>(ParameterIDs::vibeType, "Vibe",
        FrequencyTables::getVibeChoices(), 0));
    layout.add(std::make_unique<APF>(ParameterIDs::drive, "Drive",
        juce::NormalisableRange<float>(0.f, 24.f, 0.1f), 0.f));
    layout.add(std::make_unique<APF>(ParameterIDs::output, "Output",
        juce::NormalisableRange<float>(-24.f, 24.f, 0.1f), 0.f));

    return layout;
}
