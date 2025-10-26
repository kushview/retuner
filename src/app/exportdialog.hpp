// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "exporter.hpp"

namespace retuner {
namespace app {

/**
 * Dialog component for configuring export settings.
 */
class ExportDialog : public juce::Component {
public:
    ExportDialog (const juce::File& inputFile);
    ~ExportDialog() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    // Button callbacks
    std::function<void()> onExport;
    std::function<void()> onCancel;

    // Getters for user selections
    juce::File outputFile() const;
    Exporter::Quality quality() const;
    juce::String format() const;
    int bitDepth() const;
    bool shouldUpsample() const;

private:
    void browseButtonClicked();
    void updateBitDepthOptions();

    // Input file reference
    juce::File _inputFile;

    // File selection
    std::unique_ptr<juce::Label> _outputLabel;
    std::unique_ptr<juce::TextEditor> _outputPathEditor;
    std::unique_ptr<juce::TextButton> _browseButton;

    // Quality preset
    std::unique_ptr<juce::Label> _qualityLabel;
    std::unique_ptr<juce::ComboBox> _qualityCombo;

    // Format options
    std::unique_ptr<juce::Label> _formatLabel;
    std::unique_ptr<juce::ComboBox> _formatCombo;
    std::unique_ptr<juce::Label> _bitDepthLabel;
    std::unique_ptr<juce::ComboBox> _bitDepthCombo;
    std::unique_ptr<juce::ToggleButton> _upsampleToggle;

    // File chooser
    std::unique_ptr<juce::FileChooser> _fileChooser;

    // Action buttons
    std::unique_ptr<juce::TextButton> _exportButton;
    std::unique_ptr<juce::TextButton> _cancelButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExportDialog)
};

} // namespace app
} // namespace retuner
