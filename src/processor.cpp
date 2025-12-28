// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processor.hpp"
#include "editor.hpp"
#include "params.hpp"
#include "tuning.hpp"

namespace retuner {

namespace detail {
inline static void applyTuning (const Tuning& t, juce::AudioProcessorValueTreeState& s)
{
    auto sval = s.getParameterAsValue (params::SOURCE_A4_FREQUENCY);
    auto tval = s.getParameterAsValue (params::TARGET_A4_FREQUENCY);
    sval.setValue (t.sourceFrequency);
    tval.setValue (t.targetFrequency);
}
} // namespace detail

Processor::Processor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true).withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      _parameters (*this, nullptr, params::PARAMS_TYPE, createParams())
{
    _sourceA4Freq = _parameters.getRawParameterValue (params::SOURCE_A4_FREQUENCY);
    _targetA4Freq = _parameters.getRawParameterValue (params::TARGET_A4_FREQUENCY);
    _volumeDb = _parameters.getRawParameterValue (params::VOLUME_DB);
    _parameters.addParameterListener (params::VOLUME_DB, this);
    _smoothGain.reset (44100.0, 0.2);
    _smoothGain.setCurrentAndTargetValue (1.f);
}

Processor::~Processor()
{
    _parameters.removeParameterListener (params::VOLUME_DB, this);
}

juce::AudioProcessorValueTreeState::ParameterLayout Processor::createParams()
{
    using NRF = juce::NormalisableRange<float>;

    return {
        std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { params::SOURCE_A4_FREQUENCY, 1 }, "Source A4 Frequency", NRF { 380.0f, 460.0f, 0.1f }, 440.0f, juce::String(), juce::AudioProcessorParameter::genericParameter, [] (float value, int) { return juce::String (value, 1); }),
        std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { params::TARGET_A4_FREQUENCY, 1 }, "Target A4 Frequency", NRF { 380.0f, 460.0f, 0.1f }, 432.0f, juce::String(), juce::AudioProcessorParameter::genericParameter, [] (float value, int) { return juce::String (value, 1); }),
        std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { params::VOLUME_DB, 1 }, "Volume", NRF { -60.0f, 12.0f, 0.1f }, 0.0f, "dB", juce::AudioProcessorParameter::genericParameter, [] (float value, int) { return juce::String (value, 1) + " dB"; })
    };
}

void Processor::prepareToPlay (double sampleRate_, int samplesPerBlock)
{
    _sampleRate = sampleRate_;
    _samplesPerBlock = samplesPerBlock;

    // Prepare the pitch shifter and detector
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate_;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32> (juce::jmax (getTotalNumInputChannels(), getTotalNumOutputChannels()));

    _pitchShifter.prepare (spec);

    _smoothGain.reset (sampleRate_, 0.2);
    const auto gain = juce::Decibels::decibelsToGain (_parameters.getRawParameterValue (params::VOLUME_DB)->load());
    _smoothGain.setTargetValue (gain);
}

void Processor::releaseResources()
{
    _pitchShifter.reset();
}

void Processor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    const auto totalNumInputChannels = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Get current parameter values
    const auto sourceFreq = _sourceA4Freq->load();
    const auto targetFreq = _targetA4Freq->load();

    _pitchShifter.setPitchRatio (targetFreq / sourceFreq);

    // Process audio through pitch shifter
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    _pitchShifter.process (context);

    // Apply smoothed volume gain - check for target changes in a thread-safe way
    const auto targetGain = _targetGain.load();
    if (! juce::approximatelyEqual (targetGain, _smoothGain.getTargetValue())) {
        _smoothGain.setTargetValue (targetGain);
    }

    if (_smoothGain.isSmoothing()) {
        _smoothGain.applyGain (buffer, numSamples);
    } else {
        // If not smoothing, apply the current gain directly for efficiency
        buffer.applyGain (_smoothGain.getCurrentValue());
    }
}

bool Processor::hasEditor() const { return true; }
juce::AudioProcessorEditor* Processor::createEditor() { return new Editor (*this); }
const juce::String Processor::getName() const { return "reTuner"; }

bool Processor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Require both input and output to be the same
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    // Support mono and stereo
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
           || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void Processor::getStateInformation (juce::MemoryBlock& block)
{
    const auto tree = _parameters.copyState();
    juce::MemoryOutputStream ms (block, false);
    {
        juce::GZIPCompressorOutputStream gz (ms);
        tree.writeToStream (gz);
        gz.flush();
    }
}

void Processor::setStateInformation (const void* data, int size)
{
    const auto tree = juce::ValueTree::readFromGZIPData (data, static_cast<size_t> (size));
    if (tree.isValid() && tree.hasType (params::PARAMS_TYPE)) {
        _parameters.replaceState (tree);
    }
}

int Processor::getNumPrograms()
{
    return static_cast<int> (Tuning::factory().size());
}

int Processor::getCurrentProgram() { return _program; }

void Processor::setCurrentProgram (int program)
{
    auto index = static_cast<size_t> (program);
    if (program < 0 || index >= Tuning::factory().size())
        return;
    _program = program;
    detail::applyTuning (Tuning::factory()[index], _parameters);
}

const juce::String Processor::getProgramName (int program)
{
    auto index = static_cast<size_t> (program);
    if (program < 0 || index >= Tuning::factory().size())
        return "Invalid";

    return juce::String (Tuning::factory()[index].name);
}

void Processor::changeProgramName (int, const juce::String&) {}

bool Processor::acceptsMidi() const { return false; }
bool Processor::producesMidi() const { return false; }
double Processor::getTailLengthSeconds() const { return 0.0; }

void Processor::parameterChanged (const juce::String& parameterID, float newValue)
{
    if (parameterID == params::VOLUME_DB) {
        // Convert dB to linear gain and store atomically for the audio thread
        const auto gain = juce::Decibels::decibelsToGain (newValue);
        _targetGain.store (gain);
    }
}

} // namespace retuner

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new retuner::Processor();
}
