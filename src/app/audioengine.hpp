// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "../processor.hpp"

namespace retuner {
namespace app {

/**
 * Basic audio engine for the ReTuner media player.
 * Handles audio device management, file loading, and playback.
 */
class AudioEngine : public juce::AudioIODeviceCallback,
                    public juce::ChangeListener {
public:
    AudioEngine();
    ~AudioEngine() override;

    // Setup and device management
    bool initialize();
    void shutdown();

    // Audio device management
    juce::AudioDeviceManager& deviceManager() { return _deviceManager; }
    const juce::AudioDeviceManager& deviceManager() const { return _deviceManager; }

    // File loading and playback
    bool loadAudioFile (const juce::File& file);
    void play();
    void pause();
    void stop();
    bool isPlaying() const;
    bool isPaused() const;

    // Playback position control
    void setPosition (double seconds);
    double getPosition() const;
    double getDuration() const;

    // Sample rate and format info
    double getSampleRate() const;
    int getNumChannels() const;

    // Current file info
    juce::File currentFile() const;
    bool hasFileLoaded() const;
    void restoreLastLoadedFile();

    // ReTuner DSP control
    void enableReTuner (bool enabled);
    bool isReTunerEnabled() const;

    void setSourceFrequency (float frequency);
    float sourceFrequency() const noexcept;

    void setTargetFrequency (float frequency);
    float targetFrequency() const noexcept;

    retuner::Processor* processor() const { return _retunerProcessor.get(); }

    juce::AudioFormatManager& formatManager() noexcept { return _formatManager; }

    // Callbacks for UI updates
    std::function<void (double)> onPositionChanged;
    std::function<void (bool)> onPlaybackStateChanged;
    std::function<void (const juce::String&)> onErrorOccurred;
    std::function<void (const juce::File&)> onFileLoaded;

    // AudioIODeviceCallback implementation
    void audioDeviceIOCallbackWithContext (const float* const* inputChannelData,
                                           int numInputChannels,
                                           float* const* outputChannelData,
                                           int numOutputChannels,
                                           int numSamples,
                                           const juce::AudioIODeviceCallbackContext& context) override;

    void audioDeviceAboutToStart (juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;

    // ChangeListener implementation (for device changes)
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

private:
    // Core audio components
    juce::AudioDeviceManager _deviceManager;
    juce::AudioFormatManager _formatManager;
    juce::TimeSliceThread _audioFileThread;

    // Audio file playback
    std::unique_ptr<juce::AudioFormatReaderSource> _readerSource;
    std::unique_ptr<juce::AudioTransportSource> _transportSource;
    std::unique_ptr<juce::MixerAudioSource> _mixerSource;

    // ReTuner DSP processor
    std::unique_ptr<retuner::Processor> _retunerProcessor;

    // State management
    std::atomic<bool> _isInitialized { false };
    juce::File _currentFile;
    juce::String _currentFileName;

    // Audio processing thread safety
    juce::CriticalSection _callbackLock;

    // Helper methods
    void setupAudioFormats();
    void notifyError (const juce::String& message);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioEngine)
};

} // namespace app
} // namespace retuner