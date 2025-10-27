// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#include "audioengine.hpp"
#include "application.hpp"
#include "../params.hpp"

namespace retuner {
namespace app {

AudioEngine::AudioEngine()
    : _audioFileThread ("Audio File Thread")
{
    // Start the audio file thread for background audio processing
    _audioFileThread.startThread(); // Use default priority

    // Set up audio formats
    setupAudioFormats();

    // Initialize audio components
    _transportSource = std::make_unique<juce::AudioTransportSource>();
    _mixerSource = std::make_unique<juce::MixerAudioSource>();
    _retunerProcessor = std::make_unique<retuner::Processor>();

    // Attempt to restore processor state
    {
        auto& settings = Application::settingsRef();
        auto b64 = settings.processorStateBase64();
        if (b64.isNotEmpty()) {
            juce::MemoryBlock mb;
            juce::MemoryOutputStream mos (mb, false);
            if (juce::Base64::convertFromBase64 (mos, b64))
                _retunerProcessor->setStateInformation (mb.getData(), (int) mb.getSize());
        }
    }

    // Add transport source to mixer
    _mixerSource->addInputSource (_transportSource.get(), false);

    // Listen for device changes
    _deviceManager.addChangeListener (this);
}

AudioEngine::~AudioEngine()
{
    shutdown();
}

bool AudioEngine::initialize()
{
    if (_isInitialized.load())
        return true;

    // Try restoring audio device state from settings
    auto& settings = Application::settingsRef();
    std::unique_ptr<juce::XmlElement> deviceXml = settings.audioDeviceStateXML();

    // Prefer 0 inputs and 2 outputs for media playback; restore previous XML if present
    auto audioError = _deviceManager.initialise (
        0,               // Number of input channels
        2,               // Number of output channels
        deviceXml.get(), // Restore settings if available
        true             // Select default device if no settings
    );

    if (audioError.isNotEmpty()) {
        notifyError ("Failed to initialize audio device: " + audioError);
        return false;
    }

    // Re-enable synchronous audio callback registration
    _deviceManager.addAudioCallback (this);

    _isInitialized = true;

    return true;
}

void AudioEngine::shutdown()
{
    if (! _isInitialized.load())
        return;

    // Save current device state to settings
    if (auto state = _deviceManager.createStateXml()) {
        auto& settings = Application::settingsRef();
        settings.setAudioDeviceStateXML (state.get());
        settings.flush();
    }

    // Save current processor state (Base64) to settings
    if (_retunerProcessor) {
        juce::MemoryBlock mb;
        _retunerProcessor->getStateInformation (mb);
        auto b64 = juce::Base64::toBase64 (mb.getData(), mb.getSize());
        auto& settings = Application::settingsRef();
        settings.setProcessorStateBase64 (b64);
        settings.flush();
    }

    // Stop playback
    stop();

    // Remove audio callback
    _deviceManager.removeAudioCallback (this);
    _deviceManager.removeChangeListener (this);

    // Clean up audio sources
    _mixerSource->removeAllInputs();
    _transportSource.reset();
    _readerSource.reset();
    _mixerSource.reset();
    _retunerProcessor.reset();

    // Close audio device
    _deviceManager.closeAudioDevice();

    // Stop the audio file thread
    _audioFileThread.stopThread (1000);

    _isInitialized = false;
}

void AudioEngine::setupAudioFormats()
{
    // Register standard audio formats
    _formatManager.registerBasicFormats();
}

bool AudioEngine::loadAudioFile (const juce::File& file)
{
    if (! _isInitialized.load()) {
        notifyError ("Audio engine not initialized");
        return false;
    }

    // Stop current playback
    stop();

    // Try to create a reader for the file
    auto* reader = _formatManager.createReaderFor (file);

    if (reader == nullptr) {
        notifyError ("Unable to load audio file: " + file.getFileName());
        return false;
    }

    // Create a new reader source
    auto newReaderSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);

    // Replace current sources (thread-safe)
    {
        juce::ScopedLock lock (_callbackLock);

        if (_readerSource) {
            _transportSource->setSource (nullptr);
            _readerSource.reset();
        }

        _readerSource = std::move (newReaderSource);

        // Set source with proper sample rate correction
        // Pass the actual file sample rate so JUCE can resample correctly
        _transportSource->setSource (_readerSource.get(),
                                     32768,              // Read-ahead buffer for smooth playback
                                     &_audioFileThread,  // Background thread for buffering
                                     reader->sampleRate, // Source file sample rate (NOT device rate!)
                                     2);                 // Max channels
    }

    _currentFile = file;
    _currentFileName = file.getFileName();

    // Save last loaded file to settings
    auto& settings = Application::settingsRef();
    settings.setLastLoadedFile (file.getFullPathName());
    settings.flush();

    // Notify listeners that a file was loaded
    if (onFileLoaded) {
        juce::MessageManager::callAsync ([this, file]() {
            if (onFileLoaded)
                onFileLoaded (file);
        });
    }

    return true;
}

void AudioEngine::play()
{
    if (_transportSource) {
        _transportSource->start();
        if (onPlaybackStateChanged)
            onPlaybackStateChanged (true);
    }
}

void AudioEngine::pause()
{
    if (_transportSource) {
        _transportSource->stop();
        if (onPlaybackStateChanged)
            onPlaybackStateChanged (false);
    }
}

void AudioEngine::stop()
{
    if (_transportSource) {
        _transportSource->stop();
        _transportSource->setPosition (0.0);
        if (onPlaybackStateChanged)
            onPlaybackStateChanged (false);
    }
}

bool AudioEngine::isPlaying() const
{
    return _transportSource ? _transportSource->isPlaying() : false;
}

bool AudioEngine::isPaused() const
{
    return _transportSource ? ! _transportSource->isPlaying() && _transportSource->getCurrentPosition() > 0.0 : false;
}

void AudioEngine::setPosition (double seconds)
{
    if (_transportSource) {
        _transportSource->setPosition (seconds);
    }
}

double AudioEngine::getPosition() const
{
    return _transportSource ? _transportSource->getCurrentPosition() : 0.0;
}

double AudioEngine::getDuration() const
{
    return _transportSource ? _transportSource->getLengthInSeconds() : 0.0;
}

double AudioEngine::getSampleRate() const
{
    auto* device = _deviceManager.getCurrentAudioDevice();
    return device ? device->getCurrentSampleRate() : 44100.0;
}

int AudioEngine::getNumChannels() const
{
    if (_readerSource && _readerSource->getAudioFormatReader())
        return static_cast<int> (_readerSource->getAudioFormatReader()->numChannels);
    return 0;
}

void AudioEngine::audioDeviceIOCallbackWithContext (const float* const* inputChannelData,
                                                    int numInputChannels,
                                                    float* const* outputChannelData,
                                                    int numOutputChannels,
                                                    int numSamples,
                                                    const juce::AudioIODeviceCallbackContext& context)
{
    juce::ignoreUnused (inputChannelData, numInputChannels, context);

    // If there are no output channels (e.g., user selected an input-only device), nothing to do.
    if (numOutputChannels <= 0 || outputChannelData == nullptr)
        return;

    // Clear output buffers first
    for (int i = 0; i < numOutputChannels; ++i) {
        if (outputChannelData[i] != nullptr)
            juce::FloatVectorOperations::clear (outputChannelData[i], numSamples);
    }

    // Get audio from our mixer (thread-safe)
    juce::ScopedTryLock lock (_callbackLock);
    if (lock.isLocked() && _mixerSource) {
        // Safety: Ensure at least one valid output channel exists
        bool hasValidChannel = false;
        for (int i = 0; i < numOutputChannels; ++i)
            if (outputChannelData[i] != nullptr) {
                hasValidChannel = true;
                break;
            }
        if (! hasValidChannel)
            return;

        juce::AudioBuffer<float> buffer (outputChannelData, numOutputChannels, numSamples);

        juce::AudioSourceChannelInfo channelInfo;
        channelInfo.buffer = &buffer;
        channelInfo.startSample = 0;
        channelInfo.numSamples = numSamples;

        _mixerSource->getNextAudioBlock (channelInfo);

        // Process through ReTuner if enabled
        if (_retunerProcessor) {
            juce::MidiBuffer midiBuffer; // Empty MIDI buffer
            if (_retunerProcessor->isSuspended())
                _retunerProcessor->processBlockBypassed (buffer, midiBuffer);
            else
                _retunerProcessor->processBlock (buffer, midiBuffer);
        }

        // Notify position updates (occasionally, not every sample)
        static int positionUpdateCounter = 0;
        if (++positionUpdateCounter >= 1024) // Update every ~23ms at 44.1kHz
        {
            positionUpdateCounter = 0;
            if (onPositionChanged && _transportSource) {
                juce::MessageManager::callAsync ([this]() {
                    if (onPositionChanged)
                        onPositionChanged(getPosition()); });
            }
        }
    }
}

void AudioEngine::audioDeviceAboutToStart (juce::AudioIODevice* device)
{
    if (device && _mixerSource) {
        _mixerSource->prepareToPlay (device->getCurrentBufferSizeSamples(), device->getCurrentSampleRate());
    }

    // Prepare ReTuner processor
    if (_retunerProcessor && device) {
        _retunerProcessor->prepareToPlay (device->getCurrentSampleRate(), device->getCurrentBufferSizeSamples());
    }
}

void AudioEngine::audioDeviceStopped()
{
    if (_mixerSource) {
        _mixerSource->releaseResources();
    }

    // Release ReTuner processor resources
    if (_retunerProcessor) {
        _retunerProcessor->releaseResources();
    }
}

void AudioEngine::changeListenerCallback (juce::ChangeBroadcaster* source)
{
    if (source == &_deviceManager) {
        // Audio device changed - we might need to update sample rates etc.
        // For now, just restart playback if something was playing
        if (isPlaying()) {
            stop();
            // Could automatically restart playback here if desired
        }

        // Persist new device settings
        if (auto state = _deviceManager.createStateXml()) {
            auto& settings = Application::settingsRef();
            settings.setAudioDeviceStateXML (state.get());
            settings.flush();
        }
    }
}

void AudioEngine::enableReTuner (bool enabled)
{
    if (_retunerProcessor) {
        _retunerProcessor->suspendProcessing (! enabled);
    }
}

bool AudioEngine::isReTunerEnabled() const
{
    return _retunerProcessor != nullptr && ! _retunerProcessor->isSuspended();
}

void AudioEngine::setSourceFrequency (float frequency)
{
    if (_retunerProcessor)
        if (auto* p = _retunerProcessor->parameters().getParameter (retuner::params::SOURCE_A4_FREQUENCY))
            if (auto* param = dynamic_cast<juce::AudioParameterFloat*> (p))
                *param = frequency;
}

float AudioEngine::sourceFrequency() const noexcept
{
    if (_retunerProcessor)
        if (auto* p = _retunerProcessor->parameters().getParameter (retuner::params::SOURCE_A4_FREQUENCY))
            if (auto* param = dynamic_cast<juce::AudioParameterFloat*> (p))
                return param->get();
    return 440.f;
}

void AudioEngine::setTargetFrequency (float frequency)
{
    if (_retunerProcessor)
        if (auto* p = _retunerProcessor->parameters().getParameter (retuner::params::TARGET_A4_FREQUENCY))
            if (auto* param = dynamic_cast<juce::AudioParameterFloat*> (p))
                *param = frequency;
}

float AudioEngine::targetFrequency() const noexcept
{
    if (_retunerProcessor)
        if (auto* p = _retunerProcessor->parameters().getParameter (retuner::params::TARGET_A4_FREQUENCY))
            if (auto* param = dynamic_cast<juce::AudioParameterFloat*> (p))
                return param->get();
    return 432.f;
}

void AudioEngine::notifyError (const juce::String& message)
{
    if (onErrorOccurred) {
        juce::MessageManager::callAsync ([this, message]() {
            if (onErrorOccurred)
                onErrorOccurred(message); });
    }
}

juce::File AudioEngine::currentFile() const
{
    return _currentFile;
}

bool AudioEngine::hasFileLoaded() const
{
    return _currentFile.existsAsFile();
}

void AudioEngine::restoreLastLoadedFile()
{
    auto& settings = Application::settingsRef();
    auto lastFile = settings.lastLoadedFile();

    if (lastFile.isNotEmpty()) {
        juce::File file (lastFile);
        if (file.existsAsFile()) {
            loadAudioFile (file);
        }
    }
}

} // namespace app
} // namespace retuner