#pragma once

#if __has_include(<juce_gui_basics/juce_gui_basics.h>)
#include <juce_gui_basics/juce_gui_basics.h>
#elif __has_include(<JuceHeader.h>)
#include <JuceHeader.h>
#endif
#include <cmath>

//==============================================================================
// Custom LookAndFeel for pure vector round hardware knobs and warm cream pill badges
// Matches the Utali hardware faceplate style (Reverbus, Leslie, Saturator)
//==============================================================================
class UtaliLookAndFeel : public juce::LookAndFeel_V4
{
public:
    static inline juce::Colour getCreamColour() noexcept
    {
        return juce::Colour(0xffece5d8);
    }

    UtaliLookAndFeel()
    {
        const auto cream = getCreamColour();

        // Warm beige/grey and cream styling matching the hardware faceplate background
        setColour(juce::Label::textColourId, cream);
        setColour(juce::Slider::textBoxTextColourId, cream);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::thumbColourId, cream);
        setColour(juce::ComboBox::textColourId, cream);
        setColour(juce::ComboBox::arrowColourId, cream);
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff121218));
        setColour(juce::ComboBox::outlineColourId, cream.withAlpha(0.40f));
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff121218));
        setColour(juce::PopupMenu::textColourId, cream);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff2a2a34));
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& /*slider*/) override
    {
        const float margin = 5.0f;
        const float diameter = juce::jmin(static_cast<float>(width), static_cast<float>(height)) - margin * 2.0f;
        if (diameter <= 0.0f) return;

        const float centreX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
        const float centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
        const float radius = diameter * 0.5f;

        auto knobBounds = juce::Rectangle<float>(centreX - radius, centreY - radius, diameter, diameter);

        // 1. Soft ambient drop shadow for realistic 3D depth
        g.setColour(juce::Colours::black.withAlpha(0.60f));
        g.fillEllipse(knobBounds.expanded(1.5f).translated(0.0f, 2.0f));

        // 2. Outer chamfered bezel / base collar (solid metal rim)
        juce::ColourGradient bezelGrad(
            juce::Colour(0xff3c3c46), centreX, centreY - radius,
            juce::Colour(0xff121216), centreX, centreY + radius,
            false
        );
        g.setGradientFill(bezelGrad);
        g.fillEllipse(knobBounds);

        // 3. Main Knob Body: solid dark Bakelite / anodized aluminum
        auto bodyBounds = knobBounds.reduced(radius * 0.08f);
        const float bodyRadius = bodyBounds.getWidth() * 0.5f;
        juce::ColourGradient bodyGrad(
            juce::Colour(0xff2b2c34), centreX - bodyRadius * 0.3f, centreY - bodyRadius * 0.3f,
            juce::Colour(0xff141418), centreX, centreY,
            true
        );
        g.setGradientFill(bodyGrad);
        g.fillEllipse(bodyBounds);

        // Dark dividing groove
        g.setColour(juce::Colour(0xff09090c));
        g.drawEllipse(bodyBounds, 1.2f);

        // 4. Concentric inner cap
        auto innerCap = bodyBounds.reduced(bodyRadius * 0.22f);
        juce::ColourGradient capGrad(
            juce::Colour(0xff222328), centreX, centreY - innerCap.getHeight() * 0.5f,
            juce::Colour(0xff18181c), centreX, centreY + innerCap.getHeight() * 0.5f,
            false
        );
        g.setGradientFill(capGrad);
        g.fillEllipse(innerCap);
        g.setColour(juce::Colour(0xff383944).withAlpha(0.6f));
        g.drawEllipse(innerCap, 0.8f);

        // 5. Crisp pointer indicator in warm cream tone (#ece5d8)
        const float currentAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        const auto cream = getCreamColour();

        const float pointerInnerR = bodyRadius * 0.32f;
        const float pointerOuterR = bodyRadius * 0.90f;
        const float pointerWidth = juce::jmax(2.0f, diameter * 0.045f);

        const float sinA = std::sin(currentAngle);
        const float cosA = -std::cos(currentAngle);

        juce::Path pointer;
        pointer.startNewSubPath(centreX + sinA * pointerInnerR, centreY + cosA * pointerInnerR);
        pointer.lineTo(centreX + sinA * pointerOuterR, centreY + cosA * pointerOuterR);

        // Subtle drop shadow under pointer
        g.setColour(juce::Colours::black.withAlpha(0.55f));
        g.strokePath(pointer, juce::PathStrokeType(pointerWidth + 1.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Solid cream pointer
        g.setColour(cream);
        g.strokePath(pointer, juce::PathStrokeType(pointerWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Center axle dot
        g.setColour(cream.withAlpha(0.75f));
        g.fillEllipse(centreX - 1.5f, centreY - 1.5f, 3.0f, 3.0f);
    }

    void drawLabel(juce::Graphics& g, juce::Label& label) override
    {
        auto bounds = label.getLocalBounds().toFloat();
        const auto cream = getCreamColour();

        // Subtle dark pill background for guaranteed contrast against line art
        g.setColour(juce::Colour(0xdd101016));
        g.fillRoundedRectangle(bounds, 3.0f);
        g.setColour(cream.withAlpha(0.35f));
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

        g.setColour(label.findColour(juce::Label::textColourId));
        g.setFont(label.getFont());

        float fontHeight = juce::jmax(1.0f, label.getFont().getHeight());
        g.drawFittedText(label.getText(), label.getLocalBounds(), label.getJustificationType(),
                         juce::jmax(1, static_cast<int>(static_cast<float>(label.getHeight()) / fontHeight)),
                         label.getMinimumHorizontalScale());
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                      int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                      juce::ComboBox& /*box*/) override
    {
        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();
        const auto cream = getCreamColour();

        g.setColour(juce::Colour(0xdd101016));
        g.fillRoundedRectangle(bounds, 3.0f);
        g.setColour(cream.withAlpha(0.35f));
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

        // Arrow
        const float arrowSize = 4.0f;
        const float arrowX = static_cast<float>(width) - 12.0f;
        const float arrowY = static_cast<float>(height) * 0.5f;

        juce::Path p;
        p.startNewSubPath(arrowX - arrowSize, arrowY - arrowSize * 0.5f);
        p.lineTo(arrowX + arrowSize, arrowY - arrowSize * 0.5f);
        p.lineTo(arrowX, arrowY + arrowSize * 0.6f);
        p.closeSubPath();

        g.setColour(cream);
        g.fillPath(p);
    }

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds(1, 1, box.getWidth() - 20, box.getHeight() - 2);
        label.setFont(juce::FontOptions(10.5f, juce::Font::bold));
        label.setJustificationType(juce::Justification::centred);
    }
};
