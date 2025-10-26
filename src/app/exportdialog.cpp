// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#include "exportdialog.hpp"

namespace retuner {
namespace app {

ExportDialog::ExportDialog (const juce::File& inputFile)
    : _inputFile (inputFile)
{
    // Output file path
    _outputLabel = std::make_unique<juce::Label> ("outputLabel", "Output File:");
    _outputLabel->setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (_outputLabel.get());

    _outputPathEditor = std::make_unique<juce::TextEditor>();
    _outputPathEditor->setReadOnly (true);

    // Suggest default output filename with _432Hz suffix
    auto suggestedFile = _inputFile.getParentDirectory()
                             .getChildFile (_inputFile.getFileNameWithoutExtension() + "_432Hz.wav");
    _outputPathEditor->setText (suggestedFile.getFullPathName());
    addAndMakeVisible (_outputPathEditor.get());

    _browseButton = std::make_unique<juce::TextButton> ("Browse...");
    _browseButton->onClick = [this] { browseButtonClicked(); };
    addAndMakeVisible (_browseButton.get());

    // Quality preset
    _qualityLabel = std::make_unique<juce::Label> ("qualityLabel", "Quality:");
    _qualityLabel->setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (_qualityLabel.get());

    _qualityCombo = std::make_unique<juce::ComboBox>();
    _qualityCombo->addItem ("Standard Quality", 1);
    _qualityCombo->addItem ("High Quality", 2);
    _qualityCombo->addItem ("Maximum Quality", 3);
    _qualityCombo->setSelectedId (2); // Default to High Quality
    addAndMakeVisible (_qualityCombo.get());

    // Format
    _formatLabel = std::make_unique<juce::Label> ("formatLabel", "Format:");
    _formatLabel->setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (_formatLabel.get());

    _formatCombo = std::make_unique<juce::ComboBox>();
    _formatCombo->addItem ("WAV", 1);
    _formatCombo->addItem ("AIFF", 2);
    _formatCombo->setSelectedId (1); // Default to WAV
    _formatCombo->onChange = [this] { updateBitDepthOptions(); };
    addAndMakeVisible (_formatCombo.get());

    // Bit depth
    _bitDepthLabel = std::make_unique<juce::Label> ("bitDepthLabel", "Bit Depth:");
    _bitDepthLabel->setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (_bitDepthLabel.get());

    _bitDepthCombo = std::make_unique<juce::ComboBox>();
    _bitDepthCombo->addItem ("16-bit", 1);
    _bitDepthCombo->addItem ("24-bit", 2);
    _bitDepthCombo->addItem ("32-bit", 3);
    _bitDepthCombo->setSelectedId (2); // Default to 24-bit
    addAndMakeVisible (_bitDepthCombo.get());

    // Upsampling
    _upsampleToggle = std::make_unique<juce::ToggleButton> ("Enable Upsampling (96kHz)");
    _upsampleToggle->setToggleState (false, juce::dontSendNotification);
    addAndMakeVisible (_upsampleToggle.get());

    // Buttons
    _exportButton = std::make_unique<juce::TextButton> ("Export");
    _exportButton->onClick = [this]() { if (onExport) onExport(); };
    addAndMakeVisible (_exportButton.get());

    _cancelButton = std::make_unique<juce::TextButton> ("Cancel");
    _cancelButton->onClick = [this]() { if (onCancel) onCancel(); };
    addAndMakeVisible (_cancelButton.get());

    setSize (500, 280);
}

ExportDialog::~ExportDialog() = default;

void ExportDialog::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void ExportDialog::resized()
{
    auto bounds = getLocalBounds().reduced (20);
    const int labelWidth = 80;
    const int rowHeight = 30;
    const int spacing = 10;

    // Output file
    auto row = bounds.removeFromTop (rowHeight);
    _outputLabel->setBounds (row.removeFromLeft (labelWidth));
    row.removeFromLeft (spacing);
    _browseButton->setBounds (row.removeFromRight (80));
    row.removeFromRight (spacing);
    _outputPathEditor->setBounds (row);

    bounds.removeFromTop (spacing);

    // Quality
    row = bounds.removeFromTop (rowHeight);
    _qualityLabel->setBounds (row.removeFromLeft (labelWidth));
    row.removeFromLeft (spacing);
    _qualityCombo->setBounds (row);

    bounds.removeFromTop (spacing);

    // Format
    row = bounds.removeFromTop (rowHeight);
    _formatLabel->setBounds (row.removeFromLeft (labelWidth));
    row.removeFromLeft (spacing);
    _formatCombo->setBounds (row);

    bounds.removeFromTop (spacing);

    // Bit depth
    row = bounds.removeFromTop (rowHeight);
    _bitDepthLabel->setBounds (row.removeFromLeft (labelWidth));
    row.removeFromLeft (spacing);
    _bitDepthCombo->setBounds (row);

    bounds.removeFromTop (spacing);

    // Upsampling
    row = bounds.removeFromTop (rowHeight);
    _upsampleToggle->setBounds (row);

    bounds.removeFromTop (spacing * 2);

    // Buttons
    row = bounds.removeFromTop (rowHeight);
    auto buttonWidth = 80;
    _cancelButton->setBounds (row.removeFromRight (buttonWidth));
    row.removeFromRight (spacing);
    _exportButton->setBounds (row.removeFromRight (buttonWidth));
}

void ExportDialog::browseButtonClicked()
{
    auto flags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles;
    auto defaultExtension = format() == "wav" ? "*.wav" : "*.aiff";

    _fileChooser = std::make_unique<juce::FileChooser> (
        "Choose export location...",
        juce::File (_outputPathEditor->getText()),
        defaultExtension);

    _fileChooser->launchAsync (flags, [this] (const juce::FileChooser& chooser) {
        auto result = chooser.getResult();
        if (result != juce::File()) {
            // Ensure correct extension
            auto extension = format() == "wav" ? ".wav" : ".aiff";
            if (! result.hasFileExtension (extension))
                result = result.withFileExtension (extension);

            _outputPathEditor->setText (result.getFullPathName());
        }
    });
}

void ExportDialog::updateBitDepthOptions()
{
    // All options available for both WAV and AIFF
    // No changes needed currently
}

juce::File ExportDialog::outputFile() const
{
    return juce::File (_outputPathEditor->getText());
}

Exporter::Quality ExportDialog::quality() const
{
    switch (_qualityCombo->getSelectedId()) {
        case 1:
            return Exporter::Quality::Standard;
        case 2:
            return Exporter::Quality::High;
        case 3:
            return Exporter::Quality::Maximum;
        default:
            return Exporter::Quality::High;
    }
}

juce::String ExportDialog::format() const
{
    return _formatCombo->getSelectedId() == 1 ? "wav" : "aiff";
}

int ExportDialog::bitDepth() const
{
    switch (_bitDepthCombo->getSelectedId()) {
        case 1:
            return 16;
        case 2:
            return 24;
        case 3:
            return 32;
        default:
            return 24;
    }
}

bool ExportDialog::shouldUpsample() const
{
    return _upsampleToggle->getToggleState();
}

} // namespace app
} // namespace retuner
