// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "application.hpp"
#include "../style.hpp"

namespace retuner {

class Editor;

namespace app {

class MediaPlayerComponent : public juce::Component, private juce::Timer {
public:
    MediaPlayerComponent();
    ~MediaPlayerComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    // Engine accessor to integrate with outer content (e.g., device settings)
    AudioEngine* engine() const noexcept { return &Application::engineRef(); }

private:
    void timerCallback() override;
    void setupAudioEngine();
    void updateTimeDisplay();
    void updatePositionSlider();
    void formatTime (double seconds, juce::String& output);
    void updateUIForLoadedFile (const juce::File& file);

    // Button callbacks
    void playButtonClicked();
    void pauseButtonClicked();
    void stopButtonClicked();
    void loadButtonClicked();

    // Slider callbacks
    void positionSliderValueChanged();

    // Audio engine callbacks
    void onPlaybackStateChanged (bool isPlaying);
    void onPositionChanged (double position);
    void onErrorOccurred (const juce::String& error);

    // Audio engine is owned by Application

    // UI Components
    std::unique_ptr<juce::TextButton> _playButton;
    std::unique_ptr<juce::TextButton> _pauseButton;
    std::unique_ptr<juce::TextButton> _stopButton;
    std::unique_ptr<juce::TextButton> _loadButton;
    std::unique_ptr<juce::Slider> _positionSlider;
    std::unique_ptr<juce::Label> _timeLabel;
    std::unique_ptr<juce::Label> _fileLabel;

    // ReTuner Editor
    std::unique_ptr<Editor> _editor;

    // File chooser
    std::unique_ptr<juce::FileChooser> _fileChooser;

    // State tracking
    bool _isUserDraggingPosition = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MediaPlayerComponent)
};

} // namespace app
} // namespace retuner