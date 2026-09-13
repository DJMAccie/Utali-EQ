#pragma once

#if __has_include(<juce_audio_processors/juce_audio_processors.h>)
#include <juce_audio_processors/juce_audio_processors.h>
#elif __has_include(<JuceHeader.h>)
#include <JuceHeader.h>
#endif
#include <array>

namespace ParameterIDs
{
    inline constexpr const char* hpfFreq  = "HPF_FREQ";
    inline constexpr const char* lfFreq   = "LF_FREQ";
    inline constexpr const char* lfBoost  = "LF_BOOST";
    inline constexpr const char* lfAtten  = "LF_ATTEN";
    inline constexpr const char* lmfFreq  = "LMF_FREQ";
    inline constexpr const char* lmfGain  = "LMF_GAIN";
    inline constexpr const char* lmfQ     = "LMF_Q";
    inline constexpr const char* hmfFreq  = "HMF_FREQ";
    inline constexpr const char* hmfGain  = "HMF_GAIN";
    inline constexpr const char* hmfQ     = "HMF_Q";
    inline constexpr const char* hfFreq   = "HF_FREQ";
    inline constexpr const char* hfBoost  = "HF_BOOST";
    inline constexpr const char* hfAtten  = "HF_ATTEN";
    inline constexpr const char* vibeType = "VIBE_TYPE";
    inline constexpr const char* drive    = "DRIVE";
    inline constexpr const char* output   = "OUTPUT";
}

struct FrequencyTables
{
    static constexpr std::array<float, 4> lowFrequencies  { 30.0f, 60.0f, 100.0f, 200.0f };
    static constexpr std::array<float, 4> highFrequencies { 6000.0f, 8000.0f, 10000.0f, 12000.0f };

    static juce::StringArray getLowFrequencyChoices();
    static juce::StringArray getHighFrequencyChoices();
    static juce::StringArray getVibeChoices();
};

inline float getLowFrequency(int index) noexcept
{
    const auto safeIndex = juce::jlimit(0, static_cast<int>(FrequencyTables::lowFrequencies.size()) - 1, index);
    return FrequencyTables::lowFrequencies[static_cast<size_t>(safeIndex)];
}

inline float getHighFrequency(int index) noexcept
{
    const auto safeIndex = juce::jlimit(0, static_cast<int>(FrequencyTables::highFrequencies.size()) - 1, index);
    return FrequencyTables::highFrequencies[static_cast<size_t>(safeIndex)];
}

struct ParameterPointers
{
    std::atomic<float>* hpfFreq  = nullptr;
    std::atomic<float>* lfFreq   = nullptr;
    std::atomic<float>* lfBoost  = nullptr;
    std::atomic<float>* lfAtten  = nullptr;
    std::atomic<float>* lmfFreq  = nullptr;
    std::atomic<float>* lmfGain  = nullptr;
    std::atomic<float>* lmfQ     = nullptr;
    std::atomic<float>* hmfFreq  = nullptr;
    std::atomic<float>* hmfGain  = nullptr;
    std::atomic<float>* hmfQ     = nullptr;
    std::atomic<float>* hfFreq   = nullptr;
    std::atomic<float>* hfBoost  = nullptr;
    std::atomic<float>* hfAtten  = nullptr;
    std::atomic<float>* vibeType = nullptr;
    std::atomic<float>* drive    = nullptr;
    std::atomic<float>* output   = nullptr;

    void initialize(juce::AudioProcessorValueTreeState& apvts) noexcept;
};

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
