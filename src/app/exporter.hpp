// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>
#include <rubberband/RubberBandStretcher.h>

namespace retuner {
namespace app {

/**
 * High-quality audio exporter for offline frequency conversion.
 * Processes audio files with maximum quality settings using Rubber Band.
 */
class Exporter {
public:
    /** Quality preset options for export */
    enum class Quality {
        Standard, ///< Fast processing with good quality
        High,     ///< Balanced quality and speed
        Maximum   ///< Best quality, slower processing
    };

    /** Export configuration settings */
    struct ExportSettings {
        Quality quality = Quality::High;
        bool enableUpsampling = false;
        double upsampleRate = 96000.0;
        int bitDepth = 24;
        juce::String format = "wav";

        /** Creates optimal Rubber Band options based on quality setting */
        RubberBand::RubberBandStretcher::Options createRubberBandOptions() const;
    };

    /** Progress callback interface for export operations */
    struct ProgressCallback {
        std::function<void (double progress)> onProgress;
        std::function<bool()> shouldCancel;
    };

    Exporter();
    ~Exporter();

    /**
     * Export audio file with frequency conversion.
     * 
     * @param inputFile Source audio file to process
     * @param outputFile Destination file for exported audio
     * @param settings Export quality and format settings
     * @param sourceFreq Source A4 frequency (e.g. 440Hz)
     * @param targetFreq Target A4 frequency (e.g. 432Hz)
     * @param progress Optional progress callback
     * @return Result indicating success or error message
     */
    juce::Result exportAudio (const juce::File& inputFile,
                              const juce::File& outputFile,
                              const ExportSettings& settings,
                              float sourceFreq,
                              float targetFreq,
                              ProgressCallback progress = {});

    /** Get preset settings for a given quality level */
    static ExportSettings preset (Quality quality);

private:
    juce::AudioFormatManager _formatManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Exporter)
};

} // namespace app
} // namespace retuner
