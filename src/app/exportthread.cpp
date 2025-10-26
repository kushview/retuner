// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#include "exportthread.hpp"

namespace retuner {
namespace app {

ExportThread::ExportThread (const juce::File& inputFile,
                            const juce::File& outputFile,
                            const Exporter::ExportSettings& settings,
                            float sourceFreq,
                            float targetFreq)
    : juce::ThreadWithProgressWindow ("Exporting Audio...", true, true),
      _inputFile (inputFile),
      _outputFile (outputFile),
      _settings (settings),
      _sourceFreq (sourceFreq),
      _targetFreq (targetFreq),
      _result (juce::Result::ok())
{
    setStatusMessage ("Preparing export...");
}

ExportThread::~ExportThread() = default;

void ExportThread::run()
{
    // Set up progress callback
    Exporter::ProgressCallback progress;

    progress.onProgress = [this] (double p) {
        setProgress (p);

        // Update status message based on phase
        if (p < 0.5)
            setStatusMessage ("Analyzing audio (study phase)...");
        else
            setStatusMessage ("Processing audio...");
    };

    progress.shouldCancel = [this]() {
        return threadShouldExit();
    };

    // Perform the export
    _result = _exporter.exportAudio (_inputFile,
                                     _outputFile,
                                     _settings,
                                     _sourceFreq,
                                     _targetFreq,
                                     progress);
}

void ExportThread::threadComplete (bool userPressedCancel)
{
    if (userPressedCancel) {
        // Clean up partial output file if cancelled
        if (_outputFile.existsAsFile())
            _outputFile.deleteFile();

        juce::AlertWindow::showMessageBoxAsync (
            juce::MessageBoxIconType::WarningIcon,
            "Export Cancelled",
            "Export was cancelled by user.");
    } else if (_result.wasOk()) {
        juce::AlertWindow::showMessageBoxAsync (
            juce::MessageBoxIconType::InfoIcon,
            "Export Complete",
            "Audio exported successfully to:\n" + _outputFile.getFullPathName());
    } else {
        juce::AlertWindow::showMessageBoxAsync (
            juce::MessageBoxIconType::WarningIcon,
            "Export Failed",
            "Export failed with error:\n" + _result.getErrorMessage());
    }

    // Clean up - delete this thread object
    delete this;
}

} // namespace app
} // namespace retuner
