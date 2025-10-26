// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "mediaplayercomponent.hpp"

namespace retuner {
namespace app {

// A general content component that hosts the media player and provides audio device settings UI.
class ContentComponent : public juce::Component {
public:
    ContentComponent();
    ~ContentComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    std::unique_ptr<MediaPlayerComponent> _player;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ContentComponent)
};

} // namespace app
} // namespace retuner
