#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// ──────────────────────────────────────────
// Simple film‑strip Look‑and‑Feel (continuous rotary)
class FilmStripLF : public juce::LookAndFeel_V4
{
public:
    FilmStripLF(juce::Image strip, int numFrames);

    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h,
        float sliderPos, float rotaryStart, float rotaryEnd,
        juce::Slider&) override;

protected:
    juce::Image img;
    int frameW{}, frameH{}, frames{};
};

// 4‑frame “switch” version (snaps to frames‑1 stops)
class FilmStripSwitchLF : public FilmStripLF
{
public:
    FilmStripSwitchLF(juce::Image strip, int frames)
        : FilmStripLF(std::move(strip), frames) {
    }

    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h,
        float sliderPos, float rotaryStart, float rotaryEnd,
        juce::Slider&) override;
};

// ──────────────────────────────────────────
class UTALITEQAudioProcessorEditor : public juce::AudioProcessorEditor
{
    using APVTS = juce::AudioProcessorValueTreeState;
    using SliderAttach = APVTS::SliderAttachment;

public:
    explicit UTALITEQAudioProcessorEditor(UTALITEQAudioProcessor&);
    ~UTALITEQAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;

private:
    UTALITEQAudioProcessor& proc;

    // graphics
    juce::Image                         background;
    std::unique_ptr<FilmStripLF>        knobLF;
    std::unique_ptr<FilmStripSwitchLF>  freqLF;
    std::unique_ptr<FilmStripSwitchLF>  vibeLF;

    // controls
    juce::OwnedArray<juce::Slider>      knobs;
    juce::OwnedArray<SliderAttach>      attachments;

    // helpers
    static void centre(juce::Component&, int cx, int cy);
    void addControl(const juce::String& paramID, int cx, int cy);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UTALITEQAudioProcessorEditor)
};
