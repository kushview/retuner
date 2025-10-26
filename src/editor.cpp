// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#include "editor.hpp"
#include "processor.hpp"
#include "params.hpp"

namespace retuner {

Editor::Editor (Processor& p)
    : juce::AudioProcessorEditor (&p),
      _processor (p)
{
    setOpaque (true);
    setLookAndFeel (_look);

    // Title label
    _titleLabel.setText ("reTuner", juce::dontSendNotification);
    _titleLabel.setFont (juce::FontOptions().withHeight (18.0f).withStyle ("bold"));
    _titleLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (_titleLabel);

    // Placeholder combobox
    _programs.addItem ("Option 1", 1);
    _programs.addItem ("Option 2", 2);
    _programs.addItem ("Option 3", 3);
    _programs.setSelectedId (1);
    addAndMakeVisible (_programs);

    // Source frequency control
    _sourceFreqLabel.setText ("SOURCE A4", juce::dontSendNotification);
    _sourceFreqLabel.setFont (juce::FontOptions().withHeight (14.0f));
    _sourceFreqLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (_sourceFreqLabel);

    _sourceFreqSlider.setRange (380.0, 460.0, 0.1);
    _sourceFreqSlider.setValue (440.0);
    _sourceFreqSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    _sourceFreqSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    _sourceFreqSlider.onValueChange = [this] { updateSourceFreqDisplay(); };
    addAndMakeVisible (_sourceFreqSlider);

    // Source frequency digital display
    _sourceFreqDisplay.setText ("440.0", juce::dontSendNotification);
    _sourceFreqDisplay.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (_sourceFreqDisplay);

    // Target frequency control
    _targetFreqLabel.setText ("TARGET A4", juce::dontSendNotification);
    _targetFreqLabel.setFont (juce::FontOptions().withHeight (14.0f));
    _targetFreqLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (_targetFreqLabel);

    _targetFreqSlider.setRange (380.0, 460.0, 0.1);
    _targetFreqSlider.setValue (432.0);
    _targetFreqSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    _targetFreqSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    _targetFreqSlider.onValueChange = [this] { updateTargetFreqDisplay(); };
    addAndMakeVisible (_targetFreqSlider);

    // Target frequency digital display
    _targetFreqDisplay.setText ("432.0", juce::dontSendNotification);
    _targetFreqDisplay.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (_targetFreqDisplay);

    // Volume control
    _volumeLabel.setText ("VOLUME", juce::dontSendNotification);
    _volumeLabel.setFont (juce::FontOptions().withHeight (14.0f));
    _volumeLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (_volumeLabel);

    _volumeSlider.setSliderStyle (juce::Slider::LinearVertical);
    _volumeSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 60, 24);
    _volumeSlider.onValueChange = [this] { updateVolumeDisplay(); };
    addAndMakeVisible (_volumeSlider);

    // Volume digital display
    _volumeDisplay.setText ("0.0 dB", juce::dontSendNotification);
    _volumeDisplay.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (_volumeDisplay);

    // Setup colors
    setupColors();

    // Parameter attachments
    _sourceFreqAttachment = std::make_unique<SliderAttachment> (_processor.parameters, params::SOURCE_A4_FREQUENCY, _sourceFreqSlider);
    _targetFreqAttachment = std::make_unique<SliderAttachment> (_processor.parameters, params::TARGET_A4_FREQUENCY, _targetFreqSlider);
    _volumeAttachment = std::make_unique<SliderAttachment> (_processor.parameters, params::VOLUME_DB, _volumeSlider);

    // Initialize displays with current values
    updateSourceFreqDisplay();
    updateTargetFreqDisplay();
    updateVolumeDisplay();
    updatePrograms();

    setSize (404, 255); // Back to original size since volume is now on the right
}

Editor::~Editor()
{
    setLookAndFeel (nullptr);
}

void Editor::paint (juce::Graphics& g)
{
    // Modern dark background to match MediaPlayerComponent
    g.setColour (juce::Colour (0xff2d2d30)); // BACKGROUND_DARK
    g.fillAll();

    // Draw header section with subtle gradient
    const int headerHeight = 32;
    auto headerBounds = getLocalBounds().removeFromTop (headerHeight);

    // Subtle gradient for header
    juce::ColourGradient headerGradient (
        juce::Colour (0xff3a3a3d), 0.0f, static_cast<float> (headerBounds.getY()), juce::Colour (0xff2d2d30), 0.0f, static_cast<float> (headerBounds.getBottom()), false);
    g.setGradientFill (headerGradient);
    g.fillRect (headerBounds);

    // Draw separator line below header
    g.setColour (juce::Colour (0xff1a1a1c)); // Dark line
    g.drawHorizontalLine (headerBounds.getBottom(), 0.0f, static_cast<float> (getWidth()));

    // Light highlight line just below the dark line
    g.setColour (juce::Colour (0xff404040));
    g.drawHorizontalLine (headerBounds.getBottom() + 1, 0.0f, static_cast<float> (getWidth()));

    // Draw inset borders for the labels (these are drawn after header to avoid overlap)
    auto sourceLabelBounds = _sourceFreqLabel.getBounds().toFloat();
    auto targetLabelBounds = _targetFreqLabel.getBounds().toFloat();
    auto volumeLabelBounds = _volumeLabel.getBounds().toFloat();

    // Draw inset effect for labels (dark on top/left, light on bottom/right)
    auto drawInsetBorder = [&g] (juce::Rectangle<float> bounds) {
        // Dark shadow on top and left
        g.setColour (juce::Colour (0xff1a1a1c));
        g.drawLine (bounds.getX(), bounds.getY(), bounds.getRight(), bounds.getY(), 1.0f);  // Top
        g.drawLine (bounds.getX(), bounds.getY(), bounds.getX(), bounds.getBottom(), 1.0f); // Left

        // Light highlight on bottom and right
        g.setColour (juce::Colour (0xff404040));
        g.drawLine (bounds.getX(), bounds.getBottom(), bounds.getRight(), bounds.getBottom(), 1.0f); // Bottom
        g.drawLine (bounds.getRight(), bounds.getY(), bounds.getRight(), bounds.getBottom(), 1.0f);  // Right
    };

    drawInsetBorder (sourceLabelBounds);
    drawInsetBorder (targetLabelBounds);
    drawInsetBorder (volumeLabelBounds);

    // Draw digital display backgrounds
    auto sourceDisplayBounds = _sourceFreqDisplay.getBounds().toFloat();
    auto targetDisplayBounds = _targetFreqDisplay.getBounds().toFloat();
    auto volumeDisplayBounds = _volumeDisplay.getBounds().toFloat();

    // Draw subtle glow effect for digital displays
    auto drawDisplayWithGlow = [&g] (juce::Rectangle<float> bounds) {
        // Outer glow (subtle)
        g.setColour (juce::Colour (0xff4dd0e1).withAlpha (0.15f)); // ACCENT_TEAL with transparency
        g.fillRoundedRectangle (bounds.expanded (2.0f), 6.0f);

        // Inner shadow/glow
        g.setColour (juce::Colour (0xff4dd0e1).withAlpha (0.25f));
        g.fillRoundedRectangle (bounds.expanded (1.0f), 5.0f);

        // Dark background for digital displays
        g.setColour (juce::Colour (0xff1e1e20)); // BACKGROUND_DARKER
        g.fillRoundedRectangle (bounds, 4.0f);

        // Teal accent borders for digital displays
        g.setColour (juce::Colour (0xff4dd0e1)); // ACCENT_TEAL
        g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
    };

    drawDisplayWithGlow (sourceDisplayBounds);
    drawDisplayWithGlow (targetDisplayBounds);
    drawDisplayWithGlow (volumeDisplayBounds);
}

void Editor::resized()
{
    auto bounds = getLocalBounds();

    const int spacing = 10;
    const int knobSize = 120;
    const int displayHeight = 32;
    const int labelHeight = 24;
    const int headerHeight = 32; // Smaller, more compact header

    // Header area (full width)
    auto headerArea = bounds.removeFromTop (headerHeight);

    // Title centered in header (apply horizontal margins only to title)
    auto titleBounds = headerArea.reduced (26, 0);

    // Combobox on right side of header
    auto comboBounds = titleBounds.removeFromRight (120);
    comboBounds = comboBounds.withSizeKeepingCentre (120, 24);
    _programs.setBounds (comboBounds);

    // Give title the remaining space
    _titleLabel.setBounds (titleBounds);

    // Spacing below header
    bounds.removeFromTop (12);

    // Apply horizontal margins to the rest of the content
    bounds.removeFromLeft (26);
    bounds.removeFromRight (26);
    bounds.removeFromBottom (8);

    // Frequency controls layout - side by side with volume fader on right
    auto freqSection = bounds.removeFromTop (knobSize + labelHeight + displayHeight + spacing * 2);
    auto faderWidth = 90; // Width for value displays

    // Calculate even spacing for three displays across the width
    auto totalWidth = freqSection.getWidth();
    auto displaySpacing = (totalWidth - (faderWidth * 3)) / 2; // Space between displays

    // Calculate vertical positions to align all displays
    const int controlHeight = knobSize;                                     // Knobs are 120px tall
    const int displayVerticalOffset = labelHeight + 8 + controlHeight + 12; // Position where displays start

    // Source frequency (left)
    auto sourceArea = freqSection.removeFromLeft (faderWidth);
    _sourceFreqLabel.setBounds (sourceArea.removeFromTop (labelHeight));
    sourceArea.removeFromTop (8);

    auto sourceKnobArea = sourceArea.removeFromTop (knobSize);
    _sourceFreqSlider.setBounds (sourceKnobArea.withSizeKeepingCentre (knobSize, knobSize));

    sourceArea.removeFromTop (12);
    auto sourceDisplayArea = sourceArea.removeFromTop (displayHeight);
    _sourceFreqDisplay.setBounds (sourceDisplayArea.withSizeKeepingCentre (faderWidth, displayHeight));

    freqSection.removeFromLeft (displaySpacing);

    // Target frequency (middle)
    auto targetArea = freqSection.removeFromLeft (faderWidth);
    _targetFreqLabel.setBounds (targetArea.removeFromTop (labelHeight));
    targetArea.removeFromTop (8);

    auto targetKnobArea = targetArea.removeFromTop (knobSize);
    _targetFreqSlider.setBounds (targetKnobArea.withSizeKeepingCentre (knobSize, knobSize));

    targetArea.removeFromTop (12);
    auto targetDisplayArea = targetArea.removeFromTop (displayHeight);
    _targetFreqDisplay.setBounds (targetDisplayArea.withSizeKeepingCentre (faderWidth, displayHeight));

    freqSection.removeFromLeft (displaySpacing);

    // Volume fader (right)
    auto volumeArea = freqSection.removeFromLeft (faderWidth);
    _volumeLabel.setBounds (volumeArea.removeFromTop (labelHeight));
    volumeArea.removeFromTop (8);

    // Make fader shorter to prevent overlap and align display with frequency displays
    auto volumeFaderArea = volumeArea.removeFromTop (knobSize); // Same height as knobs
    _volumeSlider.setBounds (volumeFaderArea.withSizeKeepingCentre (50, knobSize));

    volumeArea.removeFromTop (12);
    auto volumeDisplayArea = volumeArea.removeFromTop (displayHeight);
    _volumeDisplay.setBounds (volumeDisplayArea.withSizeKeepingCentre (faderWidth, displayHeight));
}

void Editor::updateSourceFreqDisplay()
{
    auto value = _sourceFreqSlider.getValue();
    _sourceFreqDisplay.setText (juce::String (value, 1) + " Hz", juce::dontSendNotification);
}

void Editor::updateTargetFreqDisplay()
{
    auto value = _targetFreqSlider.getValue();
    _targetFreqDisplay.setText (juce::String (value, 1) + " Hz", juce::dontSendNotification);
}

void Editor::updateVolumeDisplay()
{
    auto value = _volumeSlider.getValue();
    _volumeDisplay.setText (juce::String (value, 1) + " dB", juce::dontSendNotification);
}

void Editor::updatePrograms()
{
    _programs.clear (juce::dontSendNotification);
    _programs.setTextWhenNothingSelected ("Program");
    const auto current = _processor.getCurrentProgram();
    for (int i = 0; i < _processor.getNumPrograms(); ++i)
        _programs.addItem (_processor.getProgramName (i), i + 1);
    _programs.setSelectedItemIndex (current, juce::dontSendNotification);
    _programs.onChange = [this]() {
        _processor.setCurrentProgram (_programs.getSelectedItemIndex());
    };
}

void Editor::setupColors()
{
    // Title label
    _titleLabel.setColour (juce::Label::textColourId, juce::Colour (0xffffffff)); // TEXT_WHITE

    // Frequency labels
    _sourceFreqLabel.setColour (juce::Label::textColourId, juce::Colour (0xffffffff)); // TEXT_WHITE
    _targetFreqLabel.setColour (juce::Label::textColourId, juce::Colour (0xffffffff)); // TEXT_WHITE
    _volumeLabel.setColour (juce::Label::textColourId, juce::Colour (0xffffffff));     // TEXT_WHITE

    // Digital displays - teal text on dark background
    _sourceFreqDisplay.setColour (juce::Label::textColourId, juce::Colour (0xff4dd0e1)); // ACCENT_TEAL
    _sourceFreqDisplay.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    _sourceFreqDisplay.setFont (juce::FontOptions().withName (juce::Font::getDefaultMonospacedFontName()).withHeight (16.0f).withStyle ("bold"));

    _targetFreqDisplay.setColour (juce::Label::textColourId, juce::Colour (0xff4dd0e1)); // ACCENT_TEAL
    _targetFreqDisplay.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    _targetFreqDisplay.setFont (juce::FontOptions().withName (juce::Font::getDefaultMonospacedFontName()).withHeight (16.0f).withStyle ("bold"));

    _volumeDisplay.setColour (juce::Label::textColourId, juce::Colour (0xff4dd0e1)); // ACCENT_TEAL
    _volumeDisplay.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    _volumeDisplay.setFont (juce::FontOptions().withName (juce::Font::getDefaultMonospacedFontName()).withHeight (16.0f).withStyle ("bold"));

    // Slider colors - these will be handled by the Style if applied
    _sourceFreqSlider.setColour (juce::Slider::thumbColourId, juce::Colour (0xff4dd0e1));      // ACCENT_TEAL
    _sourceFreqSlider.setColour (juce::Slider::trackColourId, juce::Colour (0xff404040));      // BUTTON_NORMAL
    _sourceFreqSlider.setColour (juce::Slider::backgroundColourId, juce::Colour (0xff1e1e20)); // BACKGROUND_DARKER

    _targetFreqSlider.setColour (juce::Slider::thumbColourId, juce::Colour (0xff4dd0e1));      // ACCENT_TEAL
    _targetFreqSlider.setColour (juce::Slider::trackColourId, juce::Colour (0xff404040));      // BUTTON_NORMAL
    _targetFreqSlider.setColour (juce::Slider::backgroundColourId, juce::Colour (0xff1e1e20)); // BACKGROUND_DARKER

    _volumeSlider.setColour (juce::Slider::thumbColourId, juce::Colour (0xff4dd0e1));      // ACCENT_TEAL
    _volumeSlider.setColour (juce::Slider::trackColourId, juce::Colour (0xff2d2d30));      // BACKGROUND_DARK for fader track
    _volumeSlider.setColour (juce::Slider::backgroundColourId, juce::Colour (0xff1e1e20)); // BACKGROUND_DARKER
}

} // namespace retuner
