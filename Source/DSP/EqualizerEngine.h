#pragma once

#if __has_include(<juce_dsp/juce_dsp.h>)
#include <juce_dsp/juce_dsp.h>
#elif __has_include(<JuceHeader.h>)
#include <JuceHeader.h>
#endif
#include "../Parameters.h"
#include <array>

class EqualizerEngine
{
public:
    using Filter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;

    enum FilterIndex
    {
        HighPass = 0,
        LowShelfCut,
        LowShelfBoost,
        LowMidBell,
        HighMidBell,
        HighShelfCut,
        HighShelfBoost,
        NumFilters
    };

    EqualizerEngine() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        currentSampleRate = spec.sampleRate;

        for (auto& filter : filters)
        {
            filter.prepare(spec);
            filter.reset();
        }

        outputGain.prepare(spec);
        outputGain.reset();
        outputGain.setRampDurationSeconds(0.02);

        // Invalidate cached parameters to force initial calculation on first block
        cachedParams = FilterParams{};
    }

    void reset()
    {
        for (auto& filter : filters)
            filter.reset();

        outputGain.reset();
    }

    void update(const ParameterPointers& params)
    {
        if (currentSampleRate <= 0.0)
            return;

        const auto sr = currentSampleRate;

        auto changed = [](float a, float b) noexcept {
            return std::abs(a - b) > 1e-4f;
        };

        // --- High-Pass Filter
        const float currentHpf = params.hpfFreq != nullptr ? params.hpfFreq->load(std::memory_order_relaxed) : 20.0f;
        if (changed(currentHpf, cachedParams.hpfFreq))
        {
            cachedParams.hpfFreq = currentHpf;
            *filters[HighPass].state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sr, currentHpf, 0.707f);
        }

        // --- Low Band (Shelf Cut & Shelf Boost)
        const int   currentLfIdx = params.lfFreq != nullptr ? static_cast<int>(params.lfFreq->load(std::memory_order_relaxed)) : 1;
        const float currentLfFreq = getLowFrequency(currentLfIdx);
        const float currentLfBoost = params.lfBoost != nullptr ? params.lfBoost->load(std::memory_order_relaxed) : 0.0f;
        const float currentLfAtten = params.lfAtten != nullptr ? params.lfAtten->load(std::memory_order_relaxed) : 0.0f;

        if (changed(currentLfFreq, cachedParams.lfFreq) || changed(currentLfBoost, cachedParams.lfBoost))
        {
            cachedParams.lfFreq = currentLfFreq;
            cachedParams.lfBoost = currentLfBoost;
            *filters[LowShelfBoost].state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
                sr, currentLfFreq, 0.707f, juce::Decibels::decibelsToGain(currentLfBoost));
        }

        if (changed(currentLfFreq, cachedParams.lfFreq) || changed(currentLfAtten, cachedParams.lfAtten))
        {
            cachedParams.lfFreq = currentLfFreq;
            cachedParams.lfAtten = currentLfAtten;
            *filters[LowShelfCut].state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
                sr, currentLfFreq, 0.707f, juce::Decibels::decibelsToGain(-currentLfAtten));
        }

        // --- Low-Mid Bell
        const float currentLmfFreq = params.lmfFreq != nullptr ? params.lmfFreq->load(std::memory_order_relaxed) : 700.0f;
        const float currentLmfGain = params.lmfGain != nullptr ? params.lmfGain->load(std::memory_order_relaxed) : 0.0f;
        const float currentLmfQ    = params.lmfQ != nullptr ? params.lmfQ->load(std::memory_order_relaxed) : 0.71f;

        if (changed(currentLmfFreq, cachedParams.lmfFreq) || changed(currentLmfGain, cachedParams.lmfGain) || changed(currentLmfQ, cachedParams.lmfQ))
        {
            cachedParams.lmfFreq = currentLmfFreq;
            cachedParams.lmfGain = currentLmfGain;
            cachedParams.lmfQ    = currentLmfQ;
            *filters[LowMidBell].state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                sr, currentLmfFreq, currentLmfQ, juce::Decibels::decibelsToGain(currentLmfGain));
        }

        // --- High-Mid Bell
        const float currentHmfFreq = params.hmfFreq != nullptr ? params.hmfFreq->load(std::memory_order_relaxed) : 3000.0f;
        const float currentHmfGain = params.hmfGain != nullptr ? params.hmfGain->load(std::memory_order_relaxed) : 0.0f;
        const float currentHmfQ    = params.hmfQ != nullptr ? params.hmfQ->load(std::memory_order_relaxed) : 0.71f;

        if (changed(currentHmfFreq, cachedParams.hmfFreq) || changed(currentHmfGain, cachedParams.hmfGain) || changed(currentHmfQ, cachedParams.hmfQ))
        {
            cachedParams.hmfFreq = currentHmfFreq;
            cachedParams.hmfGain = currentHmfGain;
            cachedParams.hmfQ    = currentHmfQ;
            *filters[HighMidBell].state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                sr, currentHmfFreq, currentHmfQ, juce::Decibels::decibelsToGain(currentHmfGain));
        }

        // --- High Band (Shelf Cut & Shelf Boost)
        const int   currentHfIdx = params.hfFreq != nullptr ? static_cast<int>(params.hfFreq->load(std::memory_order_relaxed)) : 2;
        const float currentHfFreq = getHighFrequency(currentHfIdx);
        const float currentHfBoost = params.hfBoost != nullptr ? params.hfBoost->load(std::memory_order_relaxed) : 0.0f;
        const float currentHfAtten = params.hfAtten != nullptr ? params.hfAtten->load(std::memory_order_relaxed) : 0.0f;

        if (changed(currentHfFreq, cachedParams.hfFreq) || changed(currentHfBoost, cachedParams.hfBoost))
        {
            cachedParams.hfFreq = currentHfFreq;
            cachedParams.hfBoost = currentHfBoost;
            *filters[HighShelfBoost].state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                sr, currentHfFreq, 0.707f, juce::Decibels::decibelsToGain(currentHfBoost));
        }

        if (changed(currentHfFreq, cachedParams.hfFreq) || changed(currentHfAtten, cachedParams.hfAtten))
        {
            cachedParams.hfFreq = currentHfFreq;
            cachedParams.hfAtten = currentHfAtten;
            *filters[HighShelfCut].state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                sr, currentHfFreq, 0.707f, juce::Decibels::decibelsToGain(-currentHfAtten));
        }

        // --- Output Gain
        const float currentOutput = params.output != nullptr ? params.output->load(std::memory_order_relaxed) : 0.0f;
        if (changed(currentOutput, cachedParams.outputDb))
        {
            cachedParams.outputDb = currentOutput;
            outputGain.setGainDecibels(currentOutput);
        }
    }

    void processEQ(juce::dsp::ProcessContextReplacing<float>& ctx) noexcept
    {
        for (auto& filter : filters)
            filter.process(ctx);
    }

    void processGain(juce::dsp::ProcessContextReplacing<float>& ctx) noexcept
    {
        outputGain.process(ctx);
    }

private:
    struct FilterParams
    {
        float hpfFreq  = -1.0f;
        float lfFreq   = -1.0f;
        float lfBoost  = -1.0f;
        float lfAtten  = -1.0f;
        float lmfFreq  = -1.0f;
        float lmfGain  = -1.0f;
        float lmfQ     = -1.0f;
        float hmfFreq  = -1.0f;
        float hmfGain  = -1.0f;
        float hmfQ     = -1.0f;
        float hfFreq   = -1.0f;
        float hfBoost  = -1.0f;
        float hfAtten  = -1.0f;
        float outputDb = -1000.0f;
    };

    double currentSampleRate = 0.0;
    FilterParams cachedParams;

    std::array<Filter, NumFilters> filters;
    juce::dsp::Gain<float> outputGain;
};
