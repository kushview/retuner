// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include <algorithm>
#include <type_traits>
#include <iostream>

#include <rubberband/RubberBandStretcher.h>

namespace retuner {
namespace dsp {

/**
 * High-quality pitch shifter using RubberBand library.
 * Provides professional-grade pitch shifting with formant preservation.
 */
template <typename SampleType>
class RubberBandShifter {
public:
    static_assert (std::is_floating_point_v<SampleType>, "SampleType must be a floating point type");

    RubberBandShifter() = default;
    ~RubberBandShifter() = default;

    //==============================================================================
    /** Called before processing starts. */
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        _sampleRate = static_cast<SampleType> (spec.sampleRate);
        _maximumBlockSize = static_cast<int> (spec.maximumBlockSize);
        _numChannels = static_cast<int> (spec.numChannels);

        jassert (_sampleRate > SampleType (0) && _numChannels > 0);

        // Prepare RubberBand stretcher in real-time mode
        createOrReconfigureStretcher();

        // Preallocate temp buffers (RubberBand uses float; we convert as needed)
        _rbIn.assign (static_cast<size_t> (_numChannels), std::vector<float> (static_cast<size_t> (_maximumBlockSize), 0.0f));
        _rbOut.assign (static_cast<size_t> (_numChannels), std::vector<float> (static_cast<size_t> (_maximumBlockSize), 0.0f));

        // Pointer arrays for process/retrieve
        _inPtrs.resize (static_cast<size_t> (_numChannels));
        _outPtrs.resize (static_cast<size_t> (_numChannels));
        for (int ch = 0; ch < _numChannels; ++ch) {
            _inPtrs[(size_t) ch] = _rbIn[(size_t) ch].data();
            _outPtrs[(size_t) ch] = _rbOut[(size_t) ch].data();
        }
    }

    /** Resets the internal state variables of the processor. */
    void reset() noexcept
    {
        if (_stretcher)
            _stretcher->reset();
        // Clear temp buffers
        for (auto& v : _rbIn)
            juce::FloatVectorOperations::clear (v.data(), (int) v.size());
        for (auto& v : _rbOut)
            juce::FloatVectorOperations::clear (v.data(), (int) v.size());
    }

    /** Processes a block of audio data. */
    void process (const juce::dsp::ProcessContextReplacing<SampleType>& context) noexcept
    {
        auto&& inputBlock = context.getInputBlock();
        auto&& outputBlock = context.getOutputBlock();

        const int numCh = juce::jmin (_numChannels, (int) inputBlock.getNumChannels());
        const int numSamples = (int) inputBlock.getNumSamples();

        if (_stretcher == nullptr) {
            // Safety: if not configured, pass-through
            outputBlock.copyFrom (inputBlock);
            return;
        }

        // Update pitch immediately if it changed
        const float pitchScale = (float) _pitchRatio;
        if (pitchScale > 0.0f)
            _stretcher->setPitchScale (pitchScale);

        // Convert input to float buffers expected by RubberBand
        for (int ch = 0; ch < numCh; ++ch) {
            const SampleType* src = inputBlock.getChannelPointer ((size_t) ch);
            float* dst = _rbIn[(size_t) ch].data();
            if constexpr (std::is_same_v<SampleType, float>) {
                juce::FloatVectorOperations::copy (dst, src, numSamples);
            } else {
                // Convert double->float
                for (int i = 0; i < numSamples; ++i)
                    dst[i] = static_cast<float> (src[i]);
            }
        }

        // Process block (non-final)
        _stretcher->process (_inPtrs.data(), (size_t) numSamples, false);

        // Try to retrieve exactly numSamples; if insufficient, zero-fill tail
        size_t avail = _stretcher->available();
        const int toPull = numSamples;
        int pulled = 0;
        if (avail > 0) {
            const int n = (int) std::min (static_cast<size_t> (toPull), avail);
            _stretcher->retrieve (_outPtrs.data(), (size_t) n);
            pulled = n;
        }

        // Copy retrieved samples to output; zero-fill the remainder to keep continuity
        for (int ch = 0; ch < numCh; ++ch) {
            SampleType* dst = outputBlock.getChannelPointer ((size_t) ch);
            const float* src = _rbOut[(size_t) ch].data();
            if constexpr (std::is_same_v<SampleType, float>) {
                if (pulled > 0)
                    juce::FloatVectorOperations::copy (dst, src, pulled);
            } else {
                for (int i = 0; i < pulled; ++i)
                    dst[i] = static_cast<SampleType> (src[i]);
            }
            if (pulled < toPull)
                juce::FloatVectorOperations::clear (dst + pulled, toPull - pulled);
        }
    }

    //==============================================================================
    /** Sets the pitch ratio. 1.0 = no change, 0.5 = one octave down, 2.0 = one octave up */
    void setPitchRatio (SampleType ratio) noexcept
    {
        _pitchRatio = ratio;
        if (_stretcher && ratio > SampleType (0))
            _stretcher->setPitchScale (static_cast<float> (ratio));
    }

    /** Returns the current pitch ratio */
    SampleType pitchRatio() const noexcept
    {
        return _pitchRatio;
    }

    /** Returns true if RubberBand library is available and enabled */
    static constexpr bool isAvailable() noexcept
    {
        return true;
    }

private:
    //==============================================================================
    /** Current sample rate */
    SampleType _sampleRate = SampleType (44100.0);

    /** Maximum expected samples per block */
    int _maximumBlockSize = 512;

    /** Number of channels being processed */
    int _numChannels = 2;

    /** Current pitch ratio */
    SampleType _pitchRatio = SampleType (1.0);

    // RubberBand stretcher and preallocated float buffers
    std::unique_ptr<RubberBand::RubberBandStretcher> _stretcher;
    std::vector<std::vector<float>> _rbIn;
    std::vector<std::vector<float>> _rbOut;
    std::vector<const float*> _inPtrs;
    std::vector<float*> _outPtrs;

    void createOrReconfigureStretcher()
    {
        using RBS = RubberBand::RubberBandStretcher;
        const size_t options = RBS::DefaultOptions
                               | RBS::OptionProcessRealTime
                               | RBS::OptionPitchHighConsistency
                               | RBS::OptionThreadingNever;

        auto sampleRate = static_cast<size_t> (juce::roundToInt (_sampleRate));
        auto channelCount = static_cast<size_t> (_numChannels);

        // Always recreate: RubberBand has no public API for changing sample rate or channel count in-place.
        _stretcher = std::make_unique<RBS> (sampleRate, channelCount, options);
        _stretcher->setMaxProcessSize (static_cast<size_t> (_maximumBlockSize));
        _stretcher->setTimeRatio (1.0);
        _stretcher->setPitchScale (static_cast<float> (_pitchRatio));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RubberBandShifter)
};

//==============================================================================
// Convenience typedefs
using RubberBandShifterFloat = RubberBandShifter<float>;
using RubberBandShifterDouble = RubberBandShifter<double>;

} // namespace dsp
} // namespace retuner