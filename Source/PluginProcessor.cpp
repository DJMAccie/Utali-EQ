#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>      // for std::tanh

//==============================================================================
UTALITEQAudioProcessor::UTALITEQAudioProcessor()
    : AudioProcessor(BusesProperties{}.withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    parameters(*this, nullptr, juce::Identifier("APVTS"), createParameterLayout())
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
UTALITEQAudioProcessor::createParameterLayout()
{
    using APF = juce::AudioParameterFloat;
    using APC = juce::AudioParameterChoice;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<APF>("HPF_FREQ", "High‑Pass Freq",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 20.f));

    layout.add(std::make_unique<APC>("LF_FREQ", "Low Freq",
        juce::StringArray{ "30", "60", "100", "200" }, 1));

    layout.add(std::make_unique<APF>("LF_BOOST", "Low Boost",
        juce::NormalisableRange<float>(0.f, 15.f, 0.1f), 0.f));
    layout.add(std::make_unique<APF>("LF_ATTEN", "Low Atten",
        juce::NormalisableRange<float>(0.f, 15.f, 0.1f), 0.f));

    layout.add(std::make_unique<APF>("LMF_FREQ", "LMF Freq",
        juce::NormalisableRange<float>(200.f, 2500.f, 1.f, 0.5f), 700.f));
    layout.add(std::make_unique<APF>("LMF_GAIN", "LMF Gain",
        juce::NormalisableRange<float>(-15.f, 15.f, 0.1f), 0.f));
    layout.add(std::make_unique<APF>("LMF_Q", "LMF Q",
        juce::NormalisableRange<float>(0.1f, 10.f, 0.01f, 0.5f), 0.71f));

    layout.add(std::make_unique<APF>("HMF_FREQ", "HMF Freq",
        juce::NormalisableRange<float>(1500.f, 7500.f, 1.f, 0.5f), 3000.f));
    layout.add(std::make_unique<APF>("HMF_GAIN", "HMF Gain",
        juce::NormalisableRange<float>(-15.f, 15.f, 0.1f), 0.f));
    layout.add(std::make_unique<APF>("HMF_Q", "HMF Q",
        juce::NormalisableRange<float>(0.1f, 10.f, 0.01f, 0.5f), 0.71f));

    layout.add(std::make_unique<APC>("HF_FREQ", "High Freq",
        juce::StringArray{ "6000", "8000", "10000", "12000" }, 2));
    layout.add(std::make_unique<APF>("HF_BOOST", "High Boost",
        juce::NormalisableRange<float>(0.f, 15.f, 0.1f), 0.f));
    layout.add(std::make_unique<APF>("HF_ATTEN", "High Atten",
        juce::NormalisableRange<float>(0.f, 15.f, 0.1f), 0.f));

    layout.add(std::make_unique<APC>("VIBE_TYPE", "Vibe",
        juce::StringArray{ "Clean", "Transistor", "Tube", "Tape" }, 0));
    layout.add(std::make_unique<APF>("DRIVE", "Drive",
        juce::NormalisableRange<float>(0.f, 24.f, 0.1f), 0.f));
    layout.add(std::make_unique<APF>("OUTPUT", "Output",
        juce::NormalisableRange<float>(-24.f, 24.f, 0.1f), 0.f));

    return layout;
}

//==============================================================================
bool UTALITEQAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet();
}

//==============================================================================
void UTALITEQAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec{ sampleRate,
                                  static_cast<juce::uint32> (samplesPerBlock),
                                  static_cast<juce::uint32> (getTotalNumOutputChannels()) };

    auto prepareFilter = [&spec](Filter& f)
        {
            f.prepare(spec);
            f.reset();
        };

    prepareFilter(highPass);
    prepareFilter(lowShelfBoost);
    prepareFilter(lowShelfCut);
    prepareFilter(lowMidBell);
    prepareFilter(highMidBell);
    prepareFilter(highShelfBoost);
    prepareFilter(highShelfCut);

    outputGain.prepare(spec);
    outputGain.reset();

    updateFilterCoefficients();
}

//==============================================================================
void UTALITEQAudioProcessor::updateFilterCoefficients()
{
    const auto sr = getSampleRate();

    // --- High‑pass
    const float hpfFreq = parameters.getRawParameterValue("HPF_FREQ")->load();
    *highPass.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sr, hpfFreq, 0.707f);

    // --- Low shelf
    const int   lfIdx = static_cast<int> (parameters.getRawParameterValue("LF_FREQ")->load());
    const float lfFreq = (lfIdx == 0 ? 30.f :
        lfIdx == 1 ? 60.f :
        lfIdx == 2 ? 100.f : 200.f);

    const float lfBoost = parameters.getRawParameterValue("LF_BOOST")->load();
    const float lfAtten = parameters.getRawParameterValue("LF_ATTEN")->load();

    *lowShelfBoost.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sr, lfFreq, 0.707f,
        juce::Decibels::decibelsToGain(lfBoost));
    *lowShelfCut.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sr, lfFreq, 0.707f,
        juce::Decibels::decibelsToGain(-lfAtten));

    // --- Low‑mid bell
    *lowMidBell.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sr,
        parameters.getRawParameterValue("LMF_FREQ")->load(),
        parameters.getRawParameterValue("LMF_Q")->load(),
        juce::Decibels::decibelsToGain(
            parameters.getRawParameterValue("LMF_GAIN")->load()));

    // --- High‑mid bell
    *highMidBell.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sr,
        parameters.getRawParameterValue("HMF_FREQ")->load(),
        parameters.getRawParameterValue("HMF_Q")->load(),
        juce::Decibels::decibelsToGain(
            parameters.getRawParameterValue("HMF_GAIN")->load()));

    // --- High shelf
    const int   hfIdx = static_cast<int> (parameters.getRawParameterValue("HF_FREQ")->load());
    const float hfFreq = (hfIdx == 0 ? 6000.f :
        hfIdx == 1 ? 8000.f :
        hfIdx == 2 ? 10000.f : 12000.f);

    const float hfBoost = parameters.getRawParameterValue("HF_BOOST")->load();
    const float hfAtten = parameters.getRawParameterValue("HF_ATTEN")->load();

    *highShelfBoost.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sr, hfFreq, 0.707f,
        juce::Decibels::decibelsToGain(hfBoost));
    *highShelfCut.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sr, hfFreq, 0.707f,
        juce::Decibels::decibelsToGain(-hfAtten));

    // --- Output gain
    outputGain.setGainDecibels(parameters.getRawParameterValue("OUTPUT")->load());
}

//==============================================================================
void UTALITEQAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer&)
{
    juce::ScopedNoDenormals _;

    if (getSampleRate() <= 0.0)
        return;

    updateFilterCoefficients();

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);

    highPass.process(ctx);
    lowShelfCut.process(ctx);
    lowShelfBoost.process(ctx);
    lowMidBell.process(ctx);
    highMidBell.process(ctx);
    highShelfCut.process(ctx);
    highShelfBoost.process(ctx);

    // --- Vibe / Drive
    const int   vibeIdx = static_cast<int> (parameters.getRawParameterValue("VIBE_TYPE")->load());
    const float driveDb = parameters.getRawParameterValue("DRIVE")->load();

    if (vibeIdx > 0 && driveDb > 0.0f)
    {
        const float drive = juce::Decibels::decibelsToGain(driveDb);

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* d = buffer.getWritePointer(ch);

            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                float x = d[i] * drive;
                switch (vibeIdx)                       // 1:Transistor 2:Tube 3:Tape
                {
                case 1: d[i] = x - (x * x * x * 0.333f); break;
                case 2: d[i] = std::tanh(x * 1.5f);     break;
                case 3: d[i] = std::tanh(x);            break;
                default: d[i] = x;                       break;
                }
            }
        }
    }

    outputGain.process(ctx);
}

//==============================================================================
void UTALITEQAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos(destData, false);
    parameters.state.writeToStream(mos);
}

void UTALITEQAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto vt = juce::ValueTree::readFromData(data, (size_t)sizeInBytes); vt.isValid())
        parameters.replaceState(vt);
}

//==============================================================================
juce::AudioProcessorEditor* UTALITEQAudioProcessor::createEditor()
{
    return new UTALITEQAudioProcessorEditor(*this);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new UTALITEQAudioProcessor();
}
