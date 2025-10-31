// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "rubberbandshifter.hpp"

namespace retuner {

class Processor : public juce::AudioProcessor,
                  public juce::AudioProcessorValueTreeState::Listener {
public:
    Processor();
    ~Processor() override;

    void prepareToPlay (double, int) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const juce::String getName() const override;

    bool isBusesLayoutSupported (const BusesLayout&) const override;

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    // AudioProcessorValueTreeState::Listener implementation
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    auto& parameters() noexcept { return _parameters; }

private:
    juce::AudioProcessorValueTreeState _parameters;
    int _program { 0 };
    double _sampleRate = 44100.0;
    int _samplesPerBlock = 512;

    // DSP Chain
    retuner::dsp::RubberBandShifter<float> _pitchShifter;

    // Parameter pointers
    std::atomic<float>* _sourceA4Freq { nullptr };
    std::atomic<float>* _targetA4Freq { nullptr };
    std::atomic<float>* _volumeDb { nullptr };

    // Cached gain value for audio thread
    std::atomic<float> _targetGain { 1.0f };
    juce::LinearSmoothedValue<float> _smoothGain;

    juce::AudioProcessorValueTreeState::ParameterLayout createParams();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Processor)
};

} // namespace retuner
