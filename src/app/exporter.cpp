// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#include "exporter.hpp"

namespace retuner {
namespace app {

//==============================================================================
Exporter::Exporter()
{
    _formatManager.registerBasicFormats();
}

Exporter::~Exporter() = default;

//==============================================================================
RubberBand::RubberBandStretcher::Options Exporter::ExportSettings::createRubberBandOptions() const
{
    using RBS = RubberBand::RubberBandStretcher;

    // Start with offline processing for best quality
    int options = RBS::OptionProcessOffline;

    // Configure based on quality preset
    switch (quality) {
        case Quality::Standard:
            options |= RBS::OptionPitchHighSpeed;
            options |= RBS::OptionWindowStandard;
            options |= RBS::OptionThreadingNever;
            break;

        case Quality::High:
            options |= RBS::OptionPitchHighConsistency;
            options |= RBS::OptionFormantPreserved;
            options |= RBS::OptionWindowStandard;
            options |= RBS::OptionSmoothingOn;
            options |= RBS::OptionThreadingAuto;
            break;

        case Quality::Maximum:
            options |= RBS::OptionPitchHighQuality;
            options |= RBS::OptionFormantPreserved;
            options |= RBS::OptionWindowLong;
            options |= RBS::OptionSmoothingOn;
            options |= RBS::OptionTransientsSmooth;
            options |= RBS::OptionDetectorSoft;
            options |= RBS::OptionPhaseLaminar;
            options |= RBS::OptionThreadingAuto;
            break;
    }

    return static_cast<RBS::Options> (options);
}

//==============================================================================
juce::Result Exporter::exportAudio (const juce::File& inputFile,
                                    const juce::File& outputFile,
                                    const ExportSettings& settings,
                                    float sourceFreq,
                                    float targetFreq,
                                    ProgressCallback progress)
{
    // Validate input
    if (! inputFile.existsAsFile())
        return juce::Result::fail ("Input file does not exist");

    if (sourceFreq <= 0.0f || targetFreq <= 0.0f)
        return juce::Result::fail ("Invalid frequency values");

    // Calculate pitch ratio
    const float pitchRatio = targetFreq / sourceFreq;

    // Open input file
    std::unique_ptr<juce::AudioFormatReader> reader (_formatManager.createReaderFor (inputFile));
    if (reader == nullptr)
        return juce::Result::fail ("Could not open input file: " + inputFile.getFullPathName());

    // Determine output sample rate
    const double outputSampleRate = settings.enableUpsampling ? settings.upsampleRate : reader->sampleRate;

    // Get output format
    juce::AudioFormat* outputFormat = nullptr;
    if (settings.format == "wav")
        outputFormat = _formatManager.findFormatForFileExtension (".wav");
    else if (settings.format == "aiff")
        outputFormat = _formatManager.findFormatForFileExtension (".aiff");

    if (outputFormat == nullptr)
        return juce::Result::fail ("Unsupported output format: " + settings.format);

    // Create output file
    std::unique_ptr<juce::OutputStream> outputStream (outputFile.createOutputStream().release());
    if (outputStream == nullptr)
        return juce::Result::fail ("Could not create output file: " + outputFile.getFullPathName());

    // Create writer options using builder pattern
    auto writerOptions = juce::AudioFormatWriterOptions {}
                             .withSampleRate (outputSampleRate)
                             .withNumChannels (static_cast<int> (reader->numChannels))
                             .withBitsPerSample (settings.bitDepth);

    // Create audio writer using modern API
    auto writer = outputFormat->createWriterFor (outputStream, writerOptions);
    if (writer == nullptr)
        return juce::Result::fail ("Could not create audio writer");

    // Create Rubber Band stretcher
    auto rbOptions = settings.createRubberBandOptions();
    RubberBand::RubberBandStretcher stretcher (
        static_cast<size_t> (outputSampleRate),
        static_cast<size_t> (reader->numChannels),
        rbOptions);

    stretcher.setTimeRatio (1.0);
    stretcher.setPitchScale (pitchRatio);

    // Set large block size for offline processing
    const int blockSize = 8192;
    stretcher.setMaxProcessSize (static_cast<size_t> (blockSize));

    // Prepare buffers and channel pointers
    juce::AudioBuffer<float> inputBuffer (static_cast<int> (reader->numChannels), blockSize);
    juce::AudioBuffer<float> outputBuffer (static_cast<int> (reader->numChannels), blockSize * 2);

    const juce::int64 totalSamples = reader->lengthInSamples;
    std::vector<const float*> inputPtrs (reader->numChannels);
    std::vector<float*> outputPtrs (reader->numChannels);

    // PASS 1: Study the entire input for optimal offline processing
    juce::int64 samplesStudied = 0;
    while (samplesStudied < totalSamples) {
        // Check for cancellation during study pass
        if (progress.shouldCancel && progress.shouldCancel())
            return juce::Result::fail ("Export cancelled by user");

        const int samplesToRead = static_cast<int> (juce::jmin<juce::int64> (blockSize, totalSamples - samplesStudied));
        reader->read (&inputBuffer, 0, samplesToRead, samplesStudied, true, true);

        // Prepare input pointers for study
        for (size_t ch = 0; ch < reader->numChannels; ++ch)
            inputPtrs[ch] = inputBuffer.getReadPointer (static_cast<int> (ch));

        // Feed to study pass (set final=true on last block)
        const bool isFinal = (samplesStudied + samplesToRead >= totalSamples);
        stretcher.study (inputPtrs.data(), static_cast<size_t> (samplesToRead), isFinal);

        samplesStudied += samplesToRead;

        // Update progress (study is first 50%)
        if (progress.onProgress) {
            const double progressPercent = 0.5 * (static_cast<double> (samplesStudied) / static_cast<double> (totalSamples));
            progress.onProgress (progressPercent);
        }
    }

    // PASS 2: Process the audio and generate output
    // Need to re-open the reader since we've already read through it
    reader.reset();
    reader.reset (_formatManager.createReaderFor (inputFile));
    if (reader == nullptr)
        return juce::Result::fail ("Could not re-open input file for processing pass");

    juce::int64 samplesProcessed = 0;
    bool finalChunk = false;

    while (samplesProcessed < totalSamples || stretcher.available() > 0) {
        // Check for cancellation during process pass
        if (progress.shouldCancel && progress.shouldCancel())
            return juce::Result::fail ("Export cancelled by user");

        // Read input chunk if not at end
        if (samplesProcessed < totalSamples) {
            const int samplesToRead = static_cast<int> (juce::jmin<juce::int64> (blockSize, totalSamples - samplesProcessed));
            reader->read (&inputBuffer, 0, samplesToRead, samplesProcessed, true, true);

            // Prepare input pointers
            for (size_t ch = 0; ch < reader->numChannels; ++ch)
                inputPtrs[ch] = inputBuffer.getReadPointer (static_cast<int> (ch));

            // Process with stretcher
            finalChunk = (samplesProcessed + samplesToRead >= totalSamples);
            stretcher.process (inputPtrs.data(), static_cast<size_t> (samplesToRead), finalChunk);

            samplesProcessed += samplesToRead;
        }

        // Retrieve processed audio
        int available = static_cast<int> (stretcher.available());
        if (available > 0) {
            available = juce::jmin (available, outputBuffer.getNumSamples());

            // Prepare output pointers
            for (size_t ch = 0; ch < reader->numChannels; ++ch)
                outputPtrs[ch] = outputBuffer.getWritePointer (static_cast<int> (ch));

            const size_t retrieved = stretcher.retrieve (outputPtrs.data(), static_cast<size_t> (available));

            // Write to output file
            if (retrieved > 0)
                writer->writeFromAudioSampleBuffer (outputBuffer, 0, static_cast<int> (retrieved));
        }

        // Update progress (process is second 50%)
        if (progress.onProgress) {
            const double progressPercent = 0.5 + 0.5 * (static_cast<double> (samplesProcessed) / static_cast<double> (totalSamples));
            progress.onProgress (progressPercent);
        }

        // Break if we've processed everything and stretcher is empty
        if (finalChunk && stretcher.available() == 0)
            break;
    }

    return juce::Result::ok();
}

//==============================================================================
Exporter::ExportSettings Exporter::preset (Quality quality)
{
    ExportSettings settings;
    settings.quality = quality;

    switch (quality) {
        case Quality::Standard:
            settings.enableUpsampling = false;
            settings.bitDepth = 16;
            break;

        case Quality::High:
            settings.enableUpsampling = false;
            settings.bitDepth = 24;
            break;

        case Quality::Maximum:
            settings.enableUpsampling = true;
            settings.upsampleRate = 96000.0;
            settings.bitDepth = 24;
            break;
    }

    return settings;
}

} // namespace app
} // namespace retuner