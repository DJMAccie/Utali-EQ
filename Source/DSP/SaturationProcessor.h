#pragma once

#if __has_include(<juce_audio_basics/juce_audio_basics.h>)
#include <juce_audio_basics/juce_audio_basics.h>
#elif __has_include(<JuceHeader.h>)
#include <JuceHeader.h>
#endif
#include <cmath>

enum class VibeType : int
{
    Clean = 0,
    Transistor = 1,
    Tube = 2,
    Tape = 3
};

inline VibeType toVibeType(int index) noexcept
{
    if (index >= 1 && index <= 3)
        return static_cast<VibeType>(index);
    return VibeType::Clean;
}

inline VibeType toVibeType(float indexFloat) noexcept
{
    return toVibeType(static_cast<int>(indexFloat));
}

class SaturationProcessor
{
public:
    SaturationProcessor() = default;

    void process(juce::AudioBuffer<float>& buffer, VibeType vibe, float driveDb) noexcept
    {
        if (vibe == VibeType::Clean || driveDb <= 0.0f)
            return;

        const float drive = juce::Decibels::decibelsToGain(driveDb);
        const int numChannels = buffer.getNumChannels();
        const int numSamples  = buffer.getNumSamples();

        // Hoist the switch outside the channel/sample loops for maximum performance and vectorization
        switch (vibe)
        {
            case VibeType::Transistor:
            {
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    auto* d = buffer.getWritePointer(ch);
                    for (int i = 0; i < numSamples; ++i)
                    {
                        const float x = d[i] * drive;
                        // Polynomial soft-clipping with bound to prevent polynomial runaway when |x| > 1.0
                        if (x > 1.0f)
                            d[i] = 2.0f / 3.0f;
                        else if (x < -1.0f)
                            d[i] = -2.0f / 3.0f;
                        else
                            d[i] = x - (x * x * x * (1.0f / 3.0f));
                    }
                }
                break;
            }

            case VibeType::Tube:
            {
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    auto* d = buffer.getWritePointer(ch);
                    for (int i = 0; i < numSamples; ++i)
                    {
                        d[i] = std::tanh(d[i] * drive * 1.5f);
                    }
                }
                break;
            }

            case VibeType::Tape:
            {
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    auto* d = buffer.getWritePointer(ch);
                    for (int i = 0; i < numSamples; ++i)
                    {
                        d[i] = std::tanh(d[i] * drive);
                    }
                }
                break;
            }

            case VibeType::Clean:
            default:
                break;
        }
    }
};
