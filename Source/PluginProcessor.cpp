#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
UTALITEQAudioProcessor::UTALITEQAudioProcessor()
    : AudioProcessor(BusesProperties{}.withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, juce::Identifier("APVTS"), createParameterLayout())
{
    paramPointers.initialize(parameters);
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
                                 static_cast<juce::uint32>(samplesPerBlock),
                                 static_cast<juce::uint32>(getTotalNumOutputChannels()) };

    equalizerEngine.prepare(spec);
}

void UTALITEQAudioProcessor::releaseResources()
{
    equalizerEngine.reset();
}

//==============================================================================
void UTALITEQAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    if (getSampleRate() <= 0.0)
        return;

    // 1. Update filter coefficients safely (dirty-checked, zero-allocation when parameters haven't changed)
    equalizerEngine.update(paramPointers);

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);

    // 2. Equalizer filters
    equalizerEngine.processEQ(ctx);

    // 3. Vibe / Saturation (hoisted switch, vectorizable loops, numerical runaway protection)
    const auto vibe = toVibeType(paramPointers.vibeType != nullptr ? paramPointers.vibeType->load(std::memory_order_relaxed) : 0.0f);
    const float driveDb = paramPointers.drive != nullptr ? paramPointers.drive->load(std::memory_order_relaxed) : 0.0f;
    saturationProcessor.process(buffer, vibe, driveDb);

    // 4. Output gain stage
    equalizerEngine.processGain(ctx);
}

//==============================================================================
void UTALITEQAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos(destData, false);
    parameters.state.writeToStream(mos);
}

void UTALITEQAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto vt = juce::ValueTree::readFromData(data, static_cast<size_t>(sizeInBytes)); vt.isValid())
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
