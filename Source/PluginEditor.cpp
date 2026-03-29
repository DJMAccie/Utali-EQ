#include "PluginEditor.h"
#include "PluginProcessor.h"

// ───────────────────────────────── Film‑strip LFs
FilmStripLF::FilmStripLF(juce::Image strip, int numFrames)
    : img(std::move(strip)), frames(numFrames)
{
    frameW = img.getWidth();
    frameH = img.getHeight() / frames;
}

void FilmStripLF::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
    float pos, float, float, juce::Slider&)
{
    const int f = juce::jlimit(0, frames - 1,
        (int)std::round(pos * (frames - 1)));

    g.drawImage(img, x, y, w, h,        // dest
        0, f * frameH, frameW, frameH);   // src
}

void FilmStripSwitchLF::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
    float pos, float s, float e, juce::Slider& sl)
{
    const int   stops = frames - 1;            // e.g. 3 → 4 positions
    const float snapped = std::round(pos * stops) / stops;
    FilmStripLF::drawRotarySlider(g, x, y, w, h, snapped, s, e, sl);
}

// ───────────────────────────────── constants + layout
static constexpr int knobPx = 40;      // rotary diameter
static constexpr int knobFrames = 100;     // frames in knob strip

static constexpr int vibeH = 40;           // vibe switch height (1:1 with knob)
static constexpr int vibeW = 91;           // preserves 221×97 aspect for 40 px tall

static constexpr int freqH = 40;          // 1‑to‑1 with knob height
static constexpr int freqW = 91;          // 221×97 strip → 40 px tall ≈ 91 px wide

/*  param‑id , centre‑x , centre‑y   (editor = 900 × 385)  */
static const struct { const char* id; int x, y; } layout[]
{
    /* Low band */ { "LF_BOOST", 215, 125 }, { "LF_ATTEN", 145, 190 }, { "LF_FREQ", 215, 190 },
    /* LMF      */ { "LMF_GAIN", 365, 125 }, { "LMF_Q",    330, 190 }, { "LMF_FREQ",400, 190 },
    /* HMF      */ { "HMF_GAIN", 535, 125 }, { "HMF_Q",    495, 190 }, { "HMF_FREQ",570, 190 },
    /* HF       */ { "HF_BOOST", 670, 125 }, { "HF_ATTEN", 750, 190 }, { "HF_FREQ", 670, 190 },
    /* Bottom   */ { "HPF_FREQ", 215, 300 }, { "DRIVE",    535, 300 }, { "OUTPUT",  670, 300 }
};

// ───────────────────────────────── helper
void UTALITEQAudioProcessorEditor::centre(juce::Component& c, int cx, int cy)
{
    c.setBounds(cx - c.getWidth() / 2,
        cy - c.getHeight() / 2,
        c.getWidth(), c.getHeight());
}

// ───────────────────────────────── constructor
UTALITEQAudioProcessorEditor::UTALITEQAudioProcessorEditor(UTALITEQAudioProcessor& p)
    : AudioProcessorEditor(&p), proc(p)
{
    /* 1 ▸ background */
    background = juce::ImageCache::getFromMemory(
        BinaryData::UTALITEQ_UI_Background_png,
        BinaryData::UTALITEQ_UI_Background_pngSize);

    /* 2 ▸ look‑and‑feels */
    auto knobStrip = juce::ImageCache::getFromMemory(
        BinaryData::knob_strip_png, BinaryData::knob_strip_pngSize);
    knobLF = std::make_unique<FilmStripLF>(knobStrip, knobFrames);

    auto freqStrip = juce::ImageCache::getFromMemory(
        BinaryData::freq_strip_png, BinaryData::freq_strip_pngSize);   // 4 frames
    freqLF = std::make_unique<FilmStripSwitchLF>(freqStrip, 4);

    auto vibeStrip = juce::ImageCache::getFromMemory(
        BinaryData::vibe_strip_png, BinaryData::vibe_strip_pngSize);   // 4 frames
    vibeLF = std::make_unique<FilmStripSwitchLF>(vibeStrip, 4);

    /* 3 ▸ rotary knobs + stepped freq switches */
    for (auto& k : layout)
        addControl(k.id, k.x, k.y);

    /* 4 ▸ vibe 4‑way switch */
    auto* vibe = new juce::Slider();
    vibe->setLookAndFeel(vibeLF.get());
    vibe->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    vibe->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    vibe->setRange(0, 3, 1);
    vibe->setBufferedToImage(true);
    vibe->setSize(vibeW, vibeH);

    addAndMakeVisible(vibe);
    attachments.add(new SliderAttach(proc.getAPVTS(), "VIBE_TYPE", *vibe));
    centre(*vibe, 365, 318);                        // tweak Y if desired

    setSize(900, 385);
}

// ───────────────────────────────── add knob / switch
void UTALITEQAudioProcessorEditor::addControl(const juce::String& id, int cx, int cy)
{
    auto* s = new juce::Slider();
    const bool stepped = (id == "LF_FREQ" || id == "HF_FREQ");

    s->setLookAndFeel(stepped ? freqLF.get() : knobLF.get());
    s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    s->setBufferedToImage(true);

    if (stepped)
    {
        s->setRange(0, 3, 1);          // 4 discrete positions
        s->setSize(freqW, freqH);      // ← give it its true aspect
    }
    else
        s->setSize(knobPx, knobPx);    // round pots

    addAndMakeVisible(s);
    centre(*s, cx, cy);

    knobs.add(s);
    attachments.add(new SliderAttach(proc.getAPVTS(), id, *s));
}

// ───────────────────────────────── paint
void UTALITEQAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.drawImage(background, getLocalBounds().toFloat());
}
