// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "exporter.hpp"

namespace retuner {
namespace app {

/**
 * Background thread that performs audio export with progress dialog.
 * Uses ThreadWithProgressWindow for automatic modal progress display.
 */
class ExportThread : public juce::ThreadWithProgressWindow {
public:
    ExportThread (const juce::File& inputFile,
                  const juce::File& outputFile,
                  const Exporter::ExportSettings& settings,
                  float sourceFreq,
                  float targetFreq);

    ~ExportThread() override;

    /** Main thread execution - performs the export */
    void run() override;

    /** Called on message thread when export completes or is cancelled */
    void threadComplete (bool userPressedCancel) override;

    /** Get the result of the export operation */
    juce::Result result() const { return _result; }

private:
    juce::File _inputFile;
    juce::File _outputFile;
    Exporter::ExportSettings _settings;
    float _sourceFreq;
    float _targetFreq;
    juce::Result _result;
    Exporter _exporter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExportThread)
};

} // namespace app
} // namespace retuner
