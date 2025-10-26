// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mediaplayercomponent.hpp"
#include "../editor.hpp"
#include "../params.hpp"

namespace retuner {
namespace app {

static constexpr int TRANSPORT_SPACE = 156;

MediaPlayerComponent::MediaPlayerComponent()
{
    // Create UI components
    _playButton = std::make_unique<juce::TextButton> ("PLAY");
    _pauseButton = std::make_unique<juce::TextButton> ("PAUSE");
    _stopButton = std::make_unique<juce::TextButton> ("STOP");
    _loadButton = std::make_unique<juce::TextButton> ("LOAD");

    _positionSlider = std::make_unique<juce::Slider> (juce::Slider::LinearHorizontal, juce::Slider::NoTextBox);

    _timeLabel = std::make_unique<juce::Label> ("Time", "00:00 / 00:00");
    _fileLabel = std::make_unique<juce::Label> ("File", "No file loaded");

    // Configure sliders
    _positionSlider->setRange (0.0, 1.0, 0.001);
    _positionSlider->setValue (0.0);
    _positionSlider->setEnabled (false);

    // Configure labels
    _timeLabel->setJustificationType (juce::Justification::centred);
    _fileLabel->setJustificationType (juce::Justification::centredLeft);

    // Configure transport buttons for modern styling
    _playButton->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff404040));
    _pauseButton->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff404040));
    _stopButton->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff404040));
    _loadButton->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff4dd0e1)); // Teal accent for load button

    // Initially disable transport buttons
    _playButton->setEnabled (false);
    _pauseButton->setEnabled (false);
    _stopButton->setEnabled (false);

    // Set up button callbacks
    _playButton->onClick = [this] { playButtonClicked(); };
    _pauseButton->onClick = [this] { pauseButtonClicked(); };
    _stopButton->onClick = [this] { stopButtonClicked(); };
    _loadButton->onClick = [this] { loadButtonClicked(); };

    // Set up slider callbacks
    _positionSlider->onValueChange = [this] { positionSliderValueChanged(); };
    _positionSlider->onDragStart = [this] { _isUserDraggingPosition = true; };
    _positionSlider->onDragEnd = [this] {
        _isUserDraggingPosition = false;
        // Commit the seek now that dragging has finished
        positionSliderValueChanged();
    };

    // Add all components
    addAndMakeVisible (_playButton.get());
    addAndMakeVisible (_pauseButton.get());
    addAndMakeVisible (_stopButton.get());
    addAndMakeVisible (_loadButton.get());
    addAndMakeVisible (_positionSlider.get());
    addAndMakeVisible (_timeLabel.get());
    addAndMakeVisible (_fileLabel.get());

    _editor = std::make_unique<Editor> (*Application::engineRef().processor());
    addAndMakeVisible (_editor.get());

    // Set up audio engine
    setupAudioEngine();

    // Check if a file is already loaded and update UI accordingly
    if (Application::engineRef().hasFileLoaded()) {
        updateUIForLoadedFile (Application::engineRef().currentFile());
    }

    setSize (_editor->getWidth(), _editor->getHeight() + TRANSPORT_SPACE);
    // Start timer for UI updates
    startTimerHz (30); // 30 FPS updates
}

MediaPlayerComponent::~MediaPlayerComponent()
{
    stopTimer();

    // Clean up look and feel
    setLookAndFeel (nullptr);

    _fileChooser = nullptr;
    _fileLabel = nullptr;
    _timeLabel = nullptr;
    _positionSlider = nullptr;
    _loadButton = nullptr;
    _stopButton = nullptr;
    _pauseButton = nullptr;
    _playButton = nullptr;
}

void MediaPlayerComponent::paint (juce::Graphics& g)
{
    Component::paint (g);
}

void MediaPlayerComponent::setupAudioEngine()
{
    auto& engine = Application::engineRef();

    // Set up callbacks
    engine.onPlaybackStateChanged = [this] (bool isPlaying) {
        onPlaybackStateChanged (isPlaying);
    };

    engine.onPositionChanged = [this] (double position) {
        onPositionChanged (position);
    };

    engine.onErrorOccurred = [this] (const juce::String& error) {
        onErrorOccurred (error);
    };

    engine.onFileLoaded = [this] (const juce::File& file) {
        // Update UI when a file is loaded
        updateUIForLoadedFile (file);
    };
}

void MediaPlayerComponent::resized()
{
    auto bounds = getLocalBounds();
    const int margin = 16;         // Increased margin for modern spacing
    const int sectionSpacing = 20; // Spacing between major sections
    const int controlSpacing = 12; // Spacing between related controls

    // ========== EDITOR SECTION (TOP) ==========
    // Give the majority of space to the embedded editor
    auto editorHeight = _editor->getHeight();
    auto editorSection = bounds.removeFromTop (editorHeight);

    if (_editor) {
        _editor->setBounds (editorSection);
    }

    bounds.removeFromTop (controlSpacing);

    // ========== TRANSPORT SECTION ==========
    auto transportSection = bounds.removeFromTop (24).reduced (margin);
    // Transport controls - centered with better spacing
    auto transportControls = transportSection;
    int buttonWidth = 60;
    int buttonHeight = 24;
    int buttonSpacing = 10;

    // Center transport controls horizontally
    auto totalTransportWidth = (buttonWidth * 4) + (buttonSpacing * 3);
    auto transportCentered = transportControls.withSizeKeepingCentre (totalTransportWidth, buttonHeight);

    _loadButton->setBounds (transportCentered.removeFromLeft (buttonWidth));
    transportCentered.removeFromLeft (buttonSpacing);
    _playButton->setBounds (transportCentered.removeFromLeft (buttonWidth));
    transportCentered.removeFromLeft (buttonSpacing);
    _pauseButton->setBounds (transportCentered.removeFromLeft (buttonWidth));
    transportCentered.removeFromLeft (buttonSpacing);
    _stopButton->setBounds (transportCentered.removeFromLeft (buttonWidth));

    bounds.removeFromTop (controlSpacing);

    // ========== MEDIA SECTION (BOTTOM) ==========
    auto mediaSection = bounds.reduced (margin);

    // Position slider and time - full width
    auto positionArea = mediaSection.removeFromTop (50); // Allocate enough space for slider + time
    _positionSlider->setBounds (positionArea.removeFromTop (28));
    _timeLabel->setBounds (positionArea.removeFromTop (22));

    mediaSection.removeFromTop (controlSpacing);

    // File info - full width
    _fileLabel->setBounds (mediaSection.removeFromTop (25));
}

void MediaPlayerComponent::timerCallback()
{
    updateTimeDisplay();
    updatePositionSlider();
}

void MediaPlayerComponent::updateTimeDisplay()
{
    double currentTime = Application::engineRef().getPosition();
    double totalTime = Application::engineRef().getDuration();

    juce::String currentStr, totalStr;
    formatTime (currentTime, currentStr);
    formatTime (totalTime, totalStr);

    _timeLabel->setText (currentStr + " / " + totalStr, juce::dontSendNotification);
}

void MediaPlayerComponent::updatePositionSlider()
{
    if (_isUserDraggingPosition)
        return;

    double duration = Application::engineRef().getDuration();
    if (duration > 0.0) {
        double position = Application::engineRef().getPosition();
        double normalizedPosition = position / duration;
        _positionSlider->setValue (normalizedPosition, juce::dontSendNotification);
    }
}

void MediaPlayerComponent::formatTime (double seconds, juce::String& output)
{
    int totalSeconds = static_cast<int> (seconds);
    int minutes = totalSeconds / 60;
    int secs = totalSeconds % 60;

    output = juce::String::formatted ("%02d:%02d", minutes, secs);
}

void MediaPlayerComponent::updateUIForLoadedFile (const juce::File& file)
{
    _fileLabel->setText (file.getFileName(), juce::dontSendNotification);
    _playButton->setEnabled (true);
    _pauseButton->setEnabled (true);
    _stopButton->setEnabled (true);
    _positionSlider->setEnabled (true);
    updateTimeDisplay();
}

void MediaPlayerComponent::playButtonClicked()
{
    Application::engineRef().play();
}

void MediaPlayerComponent::pauseButtonClicked()
{
    Application::engineRef().pause();
}

void MediaPlayerComponent::stopButtonClicked()
{
    Application::engineRef().stop();
}

void MediaPlayerComponent::loadButtonClicked()
{
    auto& formats (Application::engineRef().formatManager());

    _fileChooser = std::make_unique<juce::FileChooser> (
        "Select an audio file to play...",
        juce::File(),
        formats.getWildcardForAllFormats());

    auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    _fileChooser->launchAsync (flags, [this] (const juce::FileChooser& chooser) {
        auto file = chooser.getResult();
        if (file.existsAsFile()) {
            if (Application::engineRef().loadAudioFile (file)) {
                updateUIForLoadedFile (file);
            } else {
                juce::AlertWindow::showMessageBoxAsync (
                    juce::AlertWindow::WarningIcon,
                    "Load Error",
                    "Failed to load the selected audio file.",
                    "OK");
            }
        }
    });
}

void MediaPlayerComponent::positionSliderValueChanged()
{
    if (_isUserDraggingPosition)
        return;

    double duration = Application::engineRef().getDuration();
    if (duration > 0.0) {
        double newPosition = _positionSlider->getValue() * duration;
        Application::engineRef().setPosition (newPosition);
    }
}

void MediaPlayerComponent::onPlaybackStateChanged (bool isPlaying)
{
    // This is called from audio thread, so we need to update UI on message thread
    juce::MessageManager::callAsync ([this, isPlaying]() {
        // Update button states based on playback
        // These buttons are already enabled when a file is loaded
    });
}

void MediaPlayerComponent::onPositionChanged (double position)
{
    // Position updates are already handled by timer callback
    juce::ignoreUnused (position);
}

void MediaPlayerComponent::onErrorOccurred (const juce::String& error)
{
    juce::MessageManager::callAsync ([error]() {
        juce::AlertWindow::showMessageBoxAsync (
            juce::AlertWindow::WarningIcon,
            "Audio Error",
            error,
            "OK");
    });
}

} // namespace app
} // namespace retuner