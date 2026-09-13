#include "PluginEditor.h"

#if __has_include(<JuceHeader.h>)
#include <JuceHeader.h>
#elif __has_include("BinaryData.h")
#include "BinaryData.h"
#endif

//==============================================================================
UTALITEQAudioProcessorEditor::UTALITEQAudioProcessorEditor(UTALITEQAudioProcessor& p)
    : AudioProcessorEditor(&p), proc(p)
{
    setLookAndFeel(&utaliLookAndFeel);

    // 1. Background Artwork
#if defined(BinaryData_teq_panel_png) || defined(JUCE_USE_BINARYDATA)
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::teq_panel_png, BinaryData::teq_panel_pngSize);
#else
    // Fallback search in BinaryData or disk
    if (BinaryData::namedResourceListSize > 0)
    {
        for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
        {
            if (juce::String(BinaryData::namedResourceList[i]).containsIgnoreCase("panel")
                || juce::String(BinaryData::namedResourceList[i]).containsIgnoreCase("background"))
            {
                int sz = 0;
                auto* data = BinaryData::getNamedResource(BinaryData::namedResourceList[i], sz);
                if (data != nullptr && sz > 0)
                {
                    backgroundImage = juce::ImageCache::getFromMemory(data, sz);
                    break;
                }
            }
        }
    }
#endif

    // 2. Setup Sliders & Labels with Utali hardware vector styling
    // --- Low Band ---
    setupRotarySlider(lowBoostSlider, lowBoostLabel, "Low Boost", ParameterIDs::lfBoost, lowBoostAtt);
    lowBoostSlider.textFromValueFunction = [](double v) { return juce::String(v, 1) + " dB"; };

    setupRotarySlider(lowAttenSlider, lowAttenLabel, "Low Atten", ParameterIDs::lfAtten, lowAttenAtt);
    lowAttenSlider.textFromValueFunction = [](double v) { return juce::String(v, 1) + " dB"; };

    setupRotarySlider(lowFreqSlider, lowFreqLabel, "Low Freq", ParameterIDs::lfFreq, lowFreqAtt);
    lowFreqSlider.textFromValueFunction = [](double v) {
        const int idx = juce::jlimit(0, 3, static_cast<int>(std::round(v)));
        return FrequencyTables::getLowFrequencyChoices()[idx] + " Hz";
    };

    // --- Low-Mid Band ---
    setupRotarySlider(lmfGainSlider, lmfGainLabel, "LMF Gain", ParameterIDs::lmfGain, lmfGainAtt);
    lmfGainSlider.textFromValueFunction = [](double v) { return (v > 0 ? "+" : "") + juce::String(v, 1) + " dB"; };

    setupRotarySlider(lmfFreqSlider, lmfFreqLabel, "LMF Freq", ParameterIDs::lmfFreq, lmfFreqAtt);
    lmfFreqSlider.textFromValueFunction = [](double v) { return juce::String(static_cast<int>(std::round(v))) + " Hz"; };

    setupRotarySlider(lmfQSlider, lmfQLabel, "LMF Q", ParameterIDs::lmfQ, lmfQAtt);
    lmfQSlider.textFromValueFunction = [](double v) { return juce::String(v, 2); };

    // --- High-Mid Band ---
    setupRotarySlider(hmfGainSlider, hmfGainLabel, "HMF Gain", ParameterIDs::hmfGain, hmfGainAtt);
    hmfGainSlider.textFromValueFunction = [](double v) { return (v > 0 ? "+" : "") + juce::String(v, 1) + " dB"; };

    setupRotarySlider(hmfFreqSlider, hmfFreqLabel, "HMF Freq", ParameterIDs::hmfFreq, hmfFreqAtt);
    hmfFreqSlider.textFromValueFunction = [](double v) { return juce::String(static_cast<int>(std::round(v))) + " Hz"; };

    setupRotarySlider(hmfQSlider, hmfQLabel, "HMF Q", ParameterIDs::hmfQ, hmfQAtt);
    hmfQSlider.textFromValueFunction = [](double v) { return juce::String(v, 2); };

    // --- High Band ---
    setupRotarySlider(highBoostSlider, highBoostLabel, "High Boost", ParameterIDs::hfBoost, highBoostAtt);
    highBoostSlider.textFromValueFunction = [](double v) { return juce::String(v, 1) + " dB"; };

    setupRotarySlider(highAttenSlider, highAttenLabel, "High Atten", ParameterIDs::hfAtten, highAttenAtt);
    highAttenSlider.textFromValueFunction = [](double v) { return juce::String(v, 1) + " dB"; };

    setupRotarySlider(highFreqSlider, highFreqLabel, "High Freq", ParameterIDs::hfFreq, highFreqAtt);
    highFreqSlider.textFromValueFunction = [](double v) {
        const int idx = juce::jlimit(0, 3, static_cast<int>(std::round(v)));
        return FrequencyTables::getHighFrequencyChoices()[idx] + " Hz";
    };

    // --- Master / Character Section ---
    vibeBox.addItemList(FrequencyTables::getVibeChoices(), 1);
    vibeBox.setJustificationType(juce::Justification::centred);
    vibeBox.setLookAndFeel(&utaliLookAndFeel);
    addAndMakeVisible(vibeBox);
    vibeAtt = std::make_unique<ComboBoxAttachment>(proc.getAPVTS(), ParameterIDs::vibeType, vibeBox);

    vibeLabel.setText("Vibe Type", juce::dontSendNotification);
    vibeLabel.setJustificationType(juce::Justification::centred);
    vibeLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    vibeLabel.setColour(juce::Label::textColourId, UtaliLookAndFeel::getCreamColour());
    vibeLabel.setLookAndFeel(&utaliLookAndFeel);
    addAndMakeVisible(vibeLabel);

    setupRotarySlider(hpfSlider, hpfLabel, "High Pass", ParameterIDs::hpfFreq, hpfAtt);
    hpfSlider.textFromValueFunction = [](double v) { return juce::String(static_cast<int>(std::round(v))) + " Hz"; };

    setupRotarySlider(driveSlider, driveLabel, "Tape Drive", ParameterIDs::drive, driveAtt);
    driveSlider.textFromValueFunction = [](double v) { return juce::String(v, 1) + " dB"; };

    setupRotarySlider(outputSlider, outputLabel, "Output", ParameterIDs::output, outputAtt);
    outputSlider.textFromValueFunction = [](double v) { return (v > 0 ? "+" : "") + juce::String(v, 1) + " dB"; };

    // 3. Window sizing & fixed aspect ratio matching hardware faceplate (12439 × 5062 ≈ 2.457)
    setResizable(true, true);
    setResizeLimits(768, 313, 1536, 625);
    getConstrainer()->setFixedAspectRatio(12439.0 / 5062.0);
    setSize(1024, 417);
}

UTALITEQAudioProcessorEditor::~UTALITEQAudioProcessorEditor()
{
    // Detach all LookAndFeel pointers safely before destroying utaliLookAndFeel
    setLookAndFeel(nullptr);

    lowBoostSlider.setLookAndFeel(nullptr);
    lowAttenSlider.setLookAndFeel(nullptr);
    lowFreqSlider.setLookAndFeel(nullptr);
    lmfGainSlider.setLookAndFeel(nullptr);
    lmfFreqSlider.setLookAndFeel(nullptr);
    lmfQSlider.setLookAndFeel(nullptr);
    hmfGainSlider.setLookAndFeel(nullptr);
    hmfFreqSlider.setLookAndFeel(nullptr);
    hmfQSlider.setLookAndFeel(nullptr);
    highBoostSlider.setLookAndFeel(nullptr);
    highAttenSlider.setLookAndFeel(nullptr);
    highFreqSlider.setLookAndFeel(nullptr);
    hpfSlider.setLookAndFeel(nullptr);
    driveSlider.setLookAndFeel(nullptr);
    outputSlider.setLookAndFeel(nullptr);
    vibeBox.setLookAndFeel(nullptr);

    lowBoostLabel.setLookAndFeel(nullptr);
    lowAttenLabel.setLookAndFeel(nullptr);
    lowFreqLabel.setLookAndFeel(nullptr);
    lmfGainLabel.setLookAndFeel(nullptr);
    lmfFreqLabel.setLookAndFeel(nullptr);
    lmfQLabel.setLookAndFeel(nullptr);
    hmfGainLabel.setLookAndFeel(nullptr);
    hmfFreqLabel.setLookAndFeel(nullptr);
    hmfQLabel.setLookAndFeel(nullptr);
    highBoostLabel.setLookAndFeel(nullptr);
    highAttenLabel.setLookAndFeel(nullptr);
    highFreqLabel.setLookAndFeel(nullptr);
    hpfLabel.setLookAndFeel(nullptr);
    driveLabel.setLookAndFeel(nullptr);
    outputLabel.setLookAndFeel(nullptr);
    vibeLabel.setLookAndFeel(nullptr);
}

//==============================================================================
void UTALITEQAudioProcessorEditor::setupRotarySlider(juce::Slider& slider, juce::Label& label,
                                                     const juce::String& text, const char* paramId,
                                                     std::unique_ptr<SliderAttachment>& attachment)
{
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setLookAndFeel(&utaliLookAndFeel);
    slider.setPopupDisplayEnabled(true, true, this);
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, UtaliLookAndFeel::getCreamColour());
    label.setLookAndFeel(&utaliLookAndFeel);
    addAndMakeVisible(label);

    attachment = std::make_unique<SliderAttachment>(proc.getAPVTS(), paramId, slider);
}

//==============================================================================
void UTALITEQAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    const auto cream = UtaliLookAndFeel::getCreamColour();

    // Solid cream backing prevents alpha haloing and guarantees razor-sharp line art
    g.fillAll(cream);

    if (backgroundImage.isValid())
    {
        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
        g.drawImage(backgroundImage, bounds);
    }
    else
    {
        g.fillAll(juce::Colour(0xff22242a));
        g.setColour(juce::Colours::white);
        g.drawText("UTALI-EQ", bounds, juce::Justification::centred);
    }

    const float scale = bounds.getWidth() / 1024.0f;

    // Section Header Badges in matching warm cream (#ece5d8) and dark pill plate
    auto drawSectionBadge = [&g, scale, &cream](float cx, float cy, float w, float h, const juce::String& title) {
        auto badgeArea = juce::Rectangle<float>((cx - w * 0.5f) * scale, cy * scale, w * scale, h * scale);

        g.setColour(juce::Colour(0xdd101016));
        g.fillRoundedRectangle(badgeArea, 3.0f * scale);
        g.setColour(cream.withAlpha(0.35f));
        g.drawRoundedRectangle(badgeArea, 3.0f * scale, 1.0f);

        g.setColour(cream);
        g.setFont(juce::FontOptions(11.0f * scale, juce::Font::bold));
        g.drawText(title, badgeArea, juce::Justification::centred, false);
    };

    // Functional module centerlines positioned across the 5 faceplate columns:
    const float cx1 = 154.0f;  // Low Band
    const float cx2 = 333.0f;  // Low-Mid
    const float cx3 = 512.0f;  // High-Mid
    const float cx4 = 691.0f;  // High Band
    const float cx5 = 870.0f;  // Master / Vibe

    const float headerY = 46.0f;
    drawSectionBadge(cx1, headerY, 84.0f, 20.0f, "Low Band");
    drawSectionBadge(cx2, headerY, 84.0f, 20.0f, "Low-Mid");
    drawSectionBadge(cx3, headerY, 84.0f, 20.0f, "High-Mid");
    drawSectionBadge(cx4, headerY, 84.0f, 20.0f, "High Band");
    drawSectionBadge(cx5, headerY, 94.0f, 20.0f, "Master / Vibe");
}

//==============================================================================
void UTALITEQAudioProcessorEditor::resized()
{
    const float scale = static_cast<float>(getWidth()) / 1024.0f;
    auto S = [scale](float val) -> int {
        return static_cast<int>(std::round(val * scale));
    };

    // Module centerlines matching faceplate layout
    const int cx1 = S(154.0f);  // Low Band
    const int cx2 = S(333.0f);  // Low-Mid
    const int cx3 = S(512.0f);  // High-Mid
    const int cx4 = S(691.0f);  // High Band
    const int cx5 = S(870.0f);  // Master / Vibe

    // Helper: position a knob with its label badge placed cleanly ABOVE it
    auto placeKnob = [scale, S](juce::Slider& s, juce::Label& l, int cx, float labelY, float labelW, float knobSize) {
        const int scaledLabelW = S(labelW);
        const int scaledLabelH = S(16.0f);
        const int scaledKnob   = S(knobSize);
        const int scaledKnobH  = scaledKnob + S(6.0f); // extra height prevents drop shadow clipping
        const int scaledY      = S(labelY);

        l.setFont(juce::FontOptions(juce::jmax(8.0f, 10.0f * scale), juce::Font::bold));
        l.setBounds(cx - scaledLabelW / 2, scaledY, scaledLabelW, scaledLabelH);
        s.setBounds(cx - scaledKnob / 2, scaledY + S(17.0f), scaledKnob, scaledKnobH);
    };

    // =========================================================================
    // Column 1: Low Band (Boost, Atten, Frequency)
    // =========================================================================
    placeKnob(lowBoostSlider, lowBoostLabel, cx1, 80.0f, 76.0f, 54.0f);
    placeKnob(lowAttenSlider, lowAttenLabel, cx1, 172.0f, 76.0f, 54.0f);
    placeKnob(lowFreqSlider,  lowFreqLabel,  cx1, 264.0f, 76.0f, 54.0f);

    // =========================================================================
    // Column 2: Low-Mid (Gain, Freq, Q)
    // =========================================================================
    placeKnob(lmfGainSlider, lmfGainLabel, cx2, 80.0f, 76.0f, 54.0f);
    placeKnob(lmfFreqSlider, lmfFreqLabel, cx2, 172.0f, 76.0f, 54.0f);
    placeKnob(lmfQSlider,    lmfQLabel,    cx2, 264.0f, 76.0f, 54.0f);

    // =========================================================================
    // Column 3: High-Mid (Gain, Freq, Q)
    // =========================================================================
    placeKnob(hmfGainSlider, hmfGainLabel, cx3, 80.0f, 76.0f, 54.0f);
    placeKnob(hmfFreqSlider, hmfFreqLabel, cx3, 172.0f, 76.0f, 54.0f);
    placeKnob(hmfQSlider,    hmfQLabel,    cx3, 264.0f, 76.0f, 54.0f);

    // =========================================================================
    // Column 4: High Band (Boost, Atten, Frequency)
    // =========================================================================
    placeKnob(highBoostSlider, highBoostLabel, cx4, 80.0f, 76.0f, 54.0f);
    placeKnob(highAttenSlider, highAttenLabel, cx4, 172.0f, 76.0f, 54.0f);
    placeKnob(highFreqSlider,  highFreqLabel,  cx4, 264.0f, 76.0f, 54.0f);

    // =========================================================================
    // Column 5: Master / Vibe Section (Vibe Selector, HPF, Drive, Output)
    // =========================================================================
    const int vibeW = S(88.0f);
    vibeLabel.setFont(juce::FontOptions(juce::jmax(8.0f, 10.0f * scale), juce::Font::bold));
    vibeLabel.setBounds(cx5 - vibeW / 2, S(78.0f), vibeW, S(16.0f));
    vibeBox.setBounds(cx5 - vibeW / 2, S(96.0f), vibeW, S(22.0f));

    placeKnob(hpfSlider,    hpfLabel,    cx5, 130.0f, 74.0f, 48.0f);
    placeKnob(driveSlider,  driveLabel,  cx5, 212.0f, 76.0f, 48.0f);
    placeKnob(outputSlider, outputLabel, cx5, 294.0f, 74.0f, 48.0f);
}
