// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "style.hpp"

namespace retuner {

class Processor;

class Editor : public juce::AudioProcessorEditor {
public:
    explicit Editor (Processor& processor);
    ~Editor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    Processor& _processor;

    // UI Components
    juce::Label _titleLabel;
    juce::ComboBox _programs;

    // Source frequency control
    juce::Label _sourceFreqLabel;
    juce::Slider _sourceFreqSlider;
    juce::Label _sourceFreqDisplay;

    // Target frequency control
    juce::Label _targetFreqLabel;
    juce::Slider _targetFreqSlider;
    juce::Label _targetFreqDisplay;

    // Volume control
    juce::Label _volumeLabel;
    juce::Slider _volumeSlider;
    juce::Label _volumeDisplay;

    // Parameter attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    std::unique_ptr<SliderAttachment> _sourceFreqAttachment;
    std::unique_ptr<SliderAttachment> _targetFreqAttachment;
    std::unique_ptr<SliderAttachment> _volumeAttachment; // Helper methods

    juce::SharedResourcePointer<Style> _look;

    void updateSourceFreqDisplay();
    void updateTargetFreqDisplay();
    void updateVolumeDisplay();
    void updatePrograms();
    void setupColors();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Editor)
};

} // namespace retuner
