// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#include "contentcomponent.hpp"

namespace retuner {
namespace app {

ContentComponent::ContentComponent()
{
    setOpaque (true);
    _player = std::make_unique<MediaPlayerComponent>();
    addAndMakeVisible (_player.get());
    setSize (_player->getWidth(), _player->getHeight());
}

ContentComponent::~ContentComponent()
{
}

void ContentComponent::paint (juce::Graphics& g)
{
    // Modern dark background matching the mockup
    auto bounds = getLocalBounds().toFloat();

    // Dark gradient background
    juce::ColourGradient gradient (
        juce::Colour (0xff2d2d30), // Dark gray at top
        { bounds.getCentreX(), bounds.getCentreY() },
        juce::Colour (0xff1e1e20), // Slightly darker at bottom
        bounds.getBottomRight(),
        false);

    g.setGradientFill (gradient);
    g.fillAll();

    // Optional: Add subtle texture or noise for depth
    // This creates a very subtle texture overlay
    g.setColour (juce::Colour (0x08000000)); // Very transparent white
    for (int y = 0; y < getHeight(); y += 8) {
        g.drawHorizontalLine (y, 0.0f, static_cast<float> (getWidth()));
    }
}

void ContentComponent::resized()
{
    auto bounds = getLocalBounds();
    _player->setBounds (bounds);
}

} // namespace app
} // namespace retuner
