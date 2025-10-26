// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#include "style.hpp"

namespace retuner {

Style::Style()
{
    // Set default color scheme
    setColour (juce::ResizableWindow::backgroundColourId, juce::Colour (BACKGROUND_DARK));
    setColour (juce::TextButton::buttonColourId, juce::Colour (BUTTON_NORMAL));
    setColour (juce::TextButton::buttonOnColourId, juce::Colour (ACCENT_TEAL));
    setColour (juce::TextButton::textColourOffId, juce::Colour (TEXT_WHITE));
    setColour (juce::TextButton::textColourOnId, juce::Colour (TEXT_WHITE));
    setColour (juce::Label::textColourId, juce::Colour (TEXT_WHITE));
    setColour (juce::Slider::thumbColourId, juce::Colour (ACCENT_TEAL));
    setColour (juce::Slider::trackColourId, juce::Colour (BUTTON_NORMAL));
    setColour (juce::Slider::backgroundColourId, juce::Colour (BACKGROUND_DARKER));
    setColour (juce::ComboBox::backgroundColourId, juce::Colour (BACKGROUND_DARKER));
    setColour (juce::ComboBox::textColourId, juce::Colour (TEXT_WHITE));
    setColour (juce::ComboBox::outlineColourId, juce::Colour (BUTTON_NORMAL));
    setColour (juce::ComboBox::arrowColourId, juce::Colour (ACCENT_TEAL));

    // Popup menu colors
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (BACKGROUND_DARKER));
    setColour (juce::PopupMenu::textColourId, juce::Colour (TEXT_WHITE));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (ACCENT_TEAL).withAlpha (0.15f));
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colour (TEXT_WHITE));

    // Toggle button colors
    setColour (juce::ToggleButton::textColourId, juce::Colour (TEXT_WHITE));
    setColour (juce::ToggleButton::tickColourId, juce::Colour (ACCENT_TEAL));
    setColour (juce::ToggleButton::tickDisabledColourId, juce::Colour (BUTTON_NORMAL));

    // AlertWindow colors
    setColour (juce::AlertWindow::backgroundColourId, juce::Colour (BACKGROUND_DARK));
    setColour (juce::AlertWindow::textColourId, juce::Colour (TEXT_WHITE));
    setColour (juce::AlertWindow::outlineColourId, juce::Colour (ACCENT_TEAL));
}

void Style::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                              const juce::Colour& backgroundColour,
                                              bool shouldDrawButtonAsHighlighted,
                                              bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);

    juce::Colour fillColour = backgroundColour;

    if (shouldDrawButtonAsDown)
        fillColour = juce::Colour (BUTTON_DOWN);
    else if (shouldDrawButtonAsHighlighted)
        fillColour = juce::Colour (BUTTON_HOVER);

    // Draw rounded button with modern styling
    drawRoundedButton (g, bounds, fillColour, juce::Colour (ACCENT_TEAL));
}

void Style::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                        bool shouldDrawButtonAsHighlighted,
                                        bool shouldDrawButtonAsDown)
{
    auto font = getTextButtonFont (button, button.getHeight());
    g.setFont (font);
    g.setColour (button.findColour (button.getToggleState() ? juce::TextButton::textColourOnId
                                                            : juce::TextButton::textColourOffId)
                     .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));

    auto yIndent = juce::jmin (4, button.proportionOfHeight (0.3f));
    auto cornerSize = juce::jmin (button.getHeight(), button.getWidth()) / 2;

    auto fontHeight = juce::roundToInt (font.getHeight() * 0.6f);
    auto leftIndent = juce::jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
    auto rightIndent = juce::jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
    auto textWidth = button.getWidth() - leftIndent - rightIndent;

    if (textWidth > 0)
        g.drawFittedText (button.getButtonText(),
                          leftIndent,
                          yIndent,
                          textWidth,
                          button.getHeight() - yIndent * 2,
                          juce::Justification::centred,
                          2);
}

void Style::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                          juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (10);
    auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = juce::jmin (8.0f, radius * 0.5f);
    auto arcRadius = radius - lineW * 0.5f;

    // Draw outer ring
    juce::Path backgroundArc;
    backgroundArc.addCentredArc (bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);

    g.setColour (juce::Colour (BUTTON_NORMAL));
    g.strokePath (backgroundArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Draw value arc
    if (slider.isEnabled()) {
        juce::Path valueArc;
        valueArc.addCentredArc (bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, toAngle, true);

        g.setColour (juce::Colour (ACCENT_TEAL));
        g.strokePath (valueArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Draw center circle
    auto centerRadius = radius * 0.6f;
    g.setColour (juce::Colour (BACKGROUND_DARKER));
    g.fillEllipse (bounds.getCentreX() - centerRadius, bounds.getCentreY() - centerRadius, centerRadius * 2.0f, centerRadius * 2.0f);

    // Draw pointer
    juce::Path pointer;
    auto pointerLength = radius * 0.33f;
    auto pointerThickness = 2.0f;
    pointer.addRectangle (-pointerThickness * 0.5f, -centerRadius, pointerThickness, pointerLength);
    pointer.applyTransform (juce::AffineTransform::rotation (toAngle).translated (bounds.getCentreX(), bounds.getCentreY()));

    g.setColour (juce::Colour (ACCENT_TEAL));
    g.fillPath (pointer);
}

void Style::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float minSliderPos, float maxSliderPos,
                                          juce::Slider::SliderStyle style, juce::Slider& slider)
{
    // DJ Mixer style vertical fader
    if (style == juce::Slider::LinearVertical) {
        auto bounds = juce::Rectangle<float> (x, y, width, height);
        auto trackArea = bounds.reduced (8.0f, 4.0f);

        // Draw the fader slot (background track)
        auto slotWidth = 6.0f;
        auto slotRect = juce::Rectangle<float> (trackArea.getCentreX() - slotWidth * 0.5f,
                                                trackArea.getY(),
                                                slotWidth,
                                                trackArea.getHeight());

        g.setColour (juce::Colour (BACKGROUND_DARKER));
        g.fillRoundedRectangle (slotRect, 3.0f);

        // Draw tick marks like hardware
        g.setColour (juce::Colour (BUTTON_NORMAL));
        auto numTicks = 11; // 0, 10, 20... 100%
        for (int i = 0; i < numTicks; ++i) {
            auto tickY = trackArea.getY() + (trackArea.getHeight() * i / (numTicks - 1));
            auto tickLength = (i % 2 == 0) ? 8.0f : 4.0f; // Longer ticks at even intervals
            auto tickX = trackArea.getRight() + 2.0f;

            g.drawLine (tickX, tickY, tickX + tickLength, tickY, 1.0f);
        }

        // Use sliderPos directly like JUCE does - it's already the correct thumb center position
        auto thumbCenterY = sliderPos;

        // Draw value track (from bottom to fader position)
        // Use maxSliderPos as the bottom reference since that's where min value is
        auto valueHeight = maxSliderPos - thumbCenterY;
        if (valueHeight > 0) {
            auto valueRect = juce::Rectangle<float> (slotRect.getX(),
                                                     thumbCenterY,
                                                     slotRect.getWidth(),
                                                     valueHeight);
            g.setColour (juce::Colour (ACCENT_TEAL));
            g.fillRoundedRectangle (valueRect, 3.0f);
        }

        // Draw the fader handle
        // Use our fixed fader size
        auto faderHeight = std::max (20.f, (float) getSliderThumbRadius (slider));
        auto halfThumb = faderHeight * 0.5f;
        auto faderWidth = width - 4.0f;

        auto faderRect = juce::Rectangle<float> (bounds.getX() + 2.0f,
                                                 thumbCenterY - halfThumb,
                                                 faderWidth,
                                                 faderHeight);

        // Fader body with gradient
        juce::ColourGradient gradient (juce::Colour (0xff505050), faderRect.getTopLeft(), juce::Colour (0xff2a2a2a), faderRect.getBottomLeft(), false);
        g.setGradientFill (gradient);
        g.fillRoundedRectangle (faderRect, 4.0f);

        // Fader border
        g.setColour (juce::Colour (0xff606060));
        g.drawRoundedRectangle (faderRect, 4.0f, 1.0f);

        // Center line on fader for grip
        g.setColour (juce::Colour (0xff808080));
        auto centerY = faderRect.getCentreY();
        g.drawLine (faderRect.getX() + 4.0f, centerY, faderRect.getRight() - 4.0f, centerY, 1.0f);
    } else {
        // Fallback to standard horizontal slider
        juce::LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
    }
}

void Style::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                          bool shouldDrawButtonAsHighlighted,
                                          bool shouldDrawButtonAsDown)
{
    auto fontSize = juce::jmin (15.0f, static_cast<float> (button.getHeight()) * 0.75f);
    auto tickWidth = fontSize * 1.1f;

    drawTickBox (g, button, 4.0f, (static_cast<float> (button.getHeight()) - tickWidth) * 0.5f, tickWidth, tickWidth, button.getToggleState(), button.isEnabled(), shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

    g.setColour (button.findColour (juce::ToggleButton::textColourId));
    g.setFont (fontSize);

    if (! button.isEnabled())
        g.setOpacity (0.5f);

    g.drawFittedText (button.getButtonText(),
                      button.getLocalBounds().withTrimmedLeft (juce::roundToInt (tickWidth) + 10).withTrimmedRight (2),
                      juce::Justification::centredLeft,
                      10);
}

void Style::drawTickBox (juce::Graphics& g, juce::Component& component,
                                     float x, float y, float w, float h,
                                     bool ticked, bool isEnabled, bool shouldDrawButtonAsHighlighted,
                                     bool shouldDrawButtonAsDown)
{
    auto bounds = juce::Rectangle<float> (x, y, w, h).reduced (2.0f);
    auto cornerSize = 4.0f;

    // Background
    if (ticked) {
        // Checked state - teal background
        g.setColour (juce::Colour (ACCENT_TEAL).withAlpha (isEnabled ? 1.0f : 0.5f));
        g.fillRoundedRectangle (bounds, cornerSize);
    } else {
        // Unchecked state - dark background
        g.setColour (juce::Colour (BACKGROUND_DARKER).withAlpha (isEnabled ? 1.0f : 0.3f));
        g.fillRoundedRectangle (bounds, cornerSize);
    }

    // Border
    if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown)
        g.setColour (juce::Colour (ACCENT_TEAL).withAlpha (0.8f));
    else if (ticked)
        g.setColour (juce::Colour (ACCENT_TEAL).brighter (0.2f));
    else
        g.setColour (juce::Colour (BUTTON_NORMAL).withAlpha (isEnabled ? 1.0f : 0.3f));

    g.drawRoundedRectangle (bounds, cornerSize, 1.5f);

    // Checkmark
    if (ticked) {
        auto checkBounds = bounds.reduced (bounds.getWidth() * 0.25f);

        juce::Path checkPath;
        checkPath.startNewSubPath (checkBounds.getX(), checkBounds.getCentreY());
        checkPath.lineTo (checkBounds.getCentreX(), checkBounds.getBottom() - 2.0f);
        checkPath.lineTo (checkBounds.getRight(), checkBounds.getY() + 2.0f);

        g.setColour (juce::Colour (TEXT_WHITE).withAlpha (isEnabled ? 1.0f : 0.6f));
        g.strokePath (checkPath, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
}

void Style::drawLabel (juce::Graphics& g, juce::Label& label)
{
    g.fillAll (label.findColour (juce::Label::backgroundColourId));

    if (! label.isBeingEdited()) {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const auto textColour = label.findColour (juce::Label::textColourId).withMultipliedAlpha (alpha);

        g.setColour (textColour);
        g.setFont (label.getFont());
        g.drawFittedText (label.getText(), label.getLocalBounds(), label.getJustificationType(), juce::jmax (1, static_cast<int> (static_cast<float> (label.getHeight()) / label.getFont().getHeight())), label.getMinimumHorizontalScale());

        g.setColour (label.findColour (juce::Label::outlineColourId).withMultipliedAlpha (alpha));
    } else if (label.isEnabled()) {
        g.setColour (label.findColour (juce::Label::outlineColourId));
    }

    g.drawRect (label.getLocalBounds());
}

void Style::drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                                      int buttonX, int buttonY, int buttonW, int buttonH,
                                      juce::ComboBox& comboBox)
{
    auto cornerSize = 8.0f;
    auto bounds = juce::Rectangle<int> (0, 0, width, height).toFloat();

    // Background with dark theme
    g.setColour (juce::Colour (BACKGROUND_DARKER));
    g.fillRoundedRectangle (bounds.reduced (0.5f), cornerSize);

    // Border - use teal accent when focused/highlighted
    if (comboBox.hasKeyboardFocus (true) || isButtonDown)
        g.setColour (juce::Colour (ACCENT_TEAL));
    else
        g.setColour (juce::Colour (BUTTON_NORMAL));
    g.drawRoundedRectangle (bounds.reduced (0.5f), cornerSize, 1.0f);

    // Arrow button area with subtle background
    juce::Rectangle<int> arrowZone (width - 30, 0, 30, height);
    auto arrowBounds = arrowZone.toFloat().reduced (2.0f);

    g.setColour (juce::Colour (BUTTON_NORMAL));
    g.fillRoundedRectangle (arrowBounds, cornerSize - 2.0f);

    // Draw dropdown arrow with teal accent
    juce::Path arrow;
    auto arrowCentreX = arrowZone.getCentreX();
    auto arrowCentreY = arrowZone.getCentreY();

    arrow.startNewSubPath (arrowCentreX - 4.0f, arrowCentreY - 2.0f);
    arrow.lineTo (arrowCentreX, arrowCentreY + 2.0f);
    arrow.lineTo (arrowCentreX + 4.0f, arrowCentreY - 2.0f);

    g.setColour (juce::Colour (ACCENT_TEAL).withAlpha (comboBox.isEnabled() ? 0.9f : 0.3f));
    g.strokePath (arrow, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void Style::drawPopupMenuBackground (juce::Graphics& g, int width, int height)
{
    // Dark background with subtle border
    g.fillAll (juce::Colour (BACKGROUND_DARKER));

    // Thin teal accent border
    g.setColour (juce::Colour (ACCENT_TEAL).withAlpha (0.3f));
    g.drawRect (0, 0, width, height, 1);
}

void Style::drawPopupMenuBackgroundWithOptions (juce::Graphics& g, int width, int height,
                                                            const juce::PopupMenu::Options& options)
{
    drawPopupMenuBackground (g, width, height);
}

void Style::drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                                           bool isSeparator, bool isActive, bool isHighlighted,
                                           bool isTicked, bool hasSubMenu, const juce::String& text,
                                           const juce::String& shortcutKeyText, const juce::Drawable* icon,
                                           const juce::Colour* textColour)
{
    if (isSeparator) {
        // Draw separator line
        auto r = area.reduced (5, 0);
        r.removeFromTop (r.getHeight() / 2 - 1);

        g.setColour (juce::Colour (BUTTON_NORMAL));
        g.fillRect (r.removeFromTop (1));
    } else {
        auto r = area.reduced (1);

        // Highlight background with teal accent
        if (isHighlighted && isActive) {
            g.setColour (juce::Colour (ACCENT_TEAL).withAlpha (0.15f));
            g.fillRect (r);

            // Subtle left border accent
            g.setColour (juce::Colour (ACCENT_TEAL));
            g.fillRect (r.removeFromLeft (2));
        }

        // Text color
        auto menuTextColour = textColour != nullptr ? *textColour : juce::Colour (TEXT_WHITE);

        if (isHighlighted && isActive)
            g.setColour (juce::Colour (TEXT_WHITE));
        else
            g.setColour (menuTextColour.withMultipliedAlpha (isActive ? 1.0f : 0.5f));

        r.reduce (juce::jmin (5, area.getWidth() / 20), 0);

        auto font = juce::Font (juce::FontOptions().withHeight (area.getHeight() / 1.3f));

        if (font.getHeight() > 13.0f)
            font.setHeight (13.0f);

        g.setFont (font);

        auto textBounds = r.reduced (2);

        // Draw checkmark for ticked items
        if (isTicked) {
            auto tickBounds = textBounds.removeFromLeft (area.getHeight()).toFloat();

            juce::Path tick;
            tick.startNewSubPath (tickBounds.getX() + 4.0f, tickBounds.getCentreY());
            tick.lineTo (tickBounds.getCentreX(), tickBounds.getBottom() - 6.0f);
            tick.lineTo (tickBounds.getRight() - 4.0f, tickBounds.getY() + 4.0f);

            g.setColour (juce::Colour (ACCENT_TEAL));
            g.strokePath (tick, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // Draw submenu arrow
        if (hasSubMenu) {
            auto arrowArea = textBounds.removeFromRight (area.getHeight()).toFloat();

            juce::Path arrow;
            auto arrowCentreX = arrowArea.getCentreX();
            auto arrowCentreY = arrowArea.getCentreY();

            arrow.startNewSubPath (arrowCentreX - 2.0f, arrowCentreY - 3.0f);
            arrow.lineTo (arrowCentreX + 2.0f, arrowCentreY);
            arrow.lineTo (arrowCentreX - 2.0f, arrowCentreY + 3.0f);

            g.strokePath (arrow, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // Draw main text
        g.drawFittedText (text, textBounds, juce::Justification::centredLeft, 1);

        // Draw shortcut text
        if (shortcutKeyText.isNotEmpty()) {
            auto shortcutBounds = r.removeFromRight (
                juce::jmax (40, juce::roundToInt (juce::GlyphArrangement::getStringWidth (font, shortcutKeyText)) + 16));
            g.setColour (menuTextColour.withMultipliedAlpha (0.6f));
            g.drawFittedText (shortcutKeyText, shortcutBounds, juce::Justification::centredRight, 1);
        }
    }
}

void Style::drawRoundedButton (juce::Graphics& g, const juce::Rectangle<float>& bounds,
                                           juce::Colour fillColour, juce::Colour outlineColour,
                                           float cornerRadius) const
{
    g.setColour (fillColour);
    g.fillRoundedRectangle (bounds, cornerRadius);

    g.setColour (outlineColour);
    g.drawRoundedRectangle (bounds, cornerRadius, 1.0f);
}

void Style::drawDigitalDisplay (juce::Graphics& g, const juce::Rectangle<float>& bounds,
                                            const juce::String& text) const
{
    // Draw dark background for digital display
    g.setColour (juce::Colour (BACKGROUND_DARKER));
    g.fillRoundedRectangle (bounds, 4.0f);

    // Draw border
    g.setColour (juce::Colour (ACCENT_TEAL));
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);

    // Draw text
    g.setColour (juce::Colour (ACCENT_TEAL));
    auto font = juce::Font (juce::FontOptions()
                                .withName (juce::Font::getDefaultMonospacedFontName())
                                .withHeight (bounds.getHeight() * 0.6f)
                                .withStyle ("bold"));
    g.setFont (font);
    g.drawText (text, bounds, juce::Justification::centred, true);
}

void Style::drawAlertBox (juce::Graphics& g, juce::AlertWindow& alert,
                                      const juce::Rectangle<int>& textArea,
                                      juce::TextLayout& textLayout)
{
    auto bounds = alert.getLocalBounds().toFloat();

    // Draw dark background (square)
    g.setColour (alert.findColour (juce::AlertWindow::backgroundColourId));
    g.fillRect (bounds);

    // Draw teal accent border (square)
    g.setColour (alert.findColour (juce::AlertWindow::outlineColourId));
    g.drawRect (bounds.reduced (1.0f), 2.0f);

    // Draw icon area if present
    auto iconSpaceUsed = 0;
    auto iconWidth = 80;

    if (alert.containsAnyExtraComponents() || alert.getNumButtons() > 2)
        iconSpaceUsed = iconWidth = juce::jmin (iconWidth, static_cast<int> (bounds.getWidth() / 2));

    if (iconSpaceUsed > 0) {
        auto iconArea = bounds.removeFromLeft (static_cast<float> (iconSpaceUsed)).toNearestInt().reduced (12);

        // Draw icon background
        g.setColour (juce::Colour (BACKGROUND_DARKER));
        g.fillRect (iconArea.toFloat());

        // Draw the icon with teal color
        g.setColour (juce::Colour (ACCENT_TEAL));

        // Draw different icons based on alert type
        if (alert.getAlertType() == juce::MessageBoxIconType::WarningIcon) {
            // Warning triangle
            juce::Path warningPath;
            auto centre = iconArea.getCentre().toFloat();
            auto size = juce::jmin (iconArea.getWidth(), iconArea.getHeight()) * 0.5f;

            warningPath.addTriangle (centre.x, centre.y - size, centre.x - size * 0.866f, centre.y + size * 0.5f, centre.x + size * 0.866f, centre.y + size * 0.5f);

            g.strokePath (warningPath, juce::PathStrokeType (3.0f));

            // Exclamation mark
            g.fillEllipse (centre.x - 2.0f, centre.y + size * 0.2f, 4.0f, 4.0f);
            g.fillRoundedRectangle (centre.x - 2.0f, centre.y - size * 0.3f, 4.0f, size * 0.4f, 2.0f);
        } else if (alert.getAlertType() == juce::MessageBoxIconType::InfoIcon) {
            // Info circle with 'i'
            auto centre = iconArea.getCentre().toFloat();
            auto radius = juce::jmin (iconArea.getWidth(), iconArea.getHeight()) * 0.4f;

            g.drawEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 3.0f);

            // Draw 'i'
            g.fillEllipse (centre.x - 2.0f, centre.y - radius * 0.5f, 4.0f, 4.0f);
            g.fillRoundedRectangle (centre.x - 2.0f, centre.y - radius * 0.3f, 4.0f, radius * 0.6f, 2.0f);
        } else if (alert.getAlertType() == juce::MessageBoxIconType::QuestionIcon) {
            // Question mark
            auto centre = iconArea.getCentre().toFloat();
            auto radius = juce::jmin (iconArea.getWidth(), iconArea.getHeight()) * 0.4f;

            g.drawEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 3.0f);

            // Draw '?'
            juce::Path questionPath;
            questionPath.startNewSubPath (centre.x - radius * 0.3f, centre.y - radius * 0.3f);
            questionPath.quadraticTo (centre.x, centre.y - radius * 0.5f, centre.x, centre.y - radius * 0.1f);
            questionPath.lineTo (centre.x, centre.y + radius * 0.1f);

            g.strokePath (questionPath, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.fillEllipse (centre.x - 2.0f, centre.y + radius * 0.3f, 4.0f, 4.0f);
        }
    }

    // Draw text with proper color
    g.setColour (alert.findColour (juce::AlertWindow::textColourId));
    textLayout.draw (g, juce::Rectangle<int> (textArea.getX() + 12, textArea.getY(), textArea.getWidth() - 12, textArea.getHeight()).toFloat());
}

juce::Font Style::getAlertWindowTitleFont()
{
    return juce::Font (juce::FontOptions().withHeight (18.0f).withStyle ("bold"));
}

juce::Font Style::getAlertWindowMessageFont()
{
    return juce::Font (juce::FontOptions().withHeight (14.0f));
}

juce::Font Style::getAlertWindowFont()
{
    return juce::Font (juce::FontOptions().withHeight (14.0f));
}

} // namespace retuner
