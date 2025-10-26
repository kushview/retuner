// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace retuner {

/**
 * Modern dark theme LookAndFeel class for the ReTuner application.
 * Implements the design aesthetic shown in the UI mockup with dark backgrounds
 * and teal accent colors.
 */
class Style : public juce::LookAndFeel_V4 {
public:
    Style();
    ~Style() override = default;

    // Button styling
    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;

    // Slider styling
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override;

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle style, juce::Slider& slider) override;

    // Toggle button styling
    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool shouldDrawButtonAsHighlighted,
                           bool shouldDrawButtonAsDown) override;

    void drawTickBox (juce::Graphics& g, juce::Component& component,
                      float x, float y, float w, float h,
                      bool ticked, bool isEnabled, bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) override;

    // Label styling
    void drawLabel (juce::Graphics& g, juce::Label& label) override;

    // ComboBox styling
    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& comboBox) override;

    // Popup menu styling
    void drawPopupMenuBackground (juce::Graphics& g, int width, int height) override;

    void drawPopupMenuBackgroundWithOptions (juce::Graphics& g, int width, int height,
                                             const juce::PopupMenu::Options& options) override;

    void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                            bool isSeparator, bool isActive, bool isHighlighted,
                            bool isTicked, bool hasSubMenu, const juce::String& text,
                            const juce::String& shortcutKeyText, const juce::Drawable* icon,
                            const juce::Colour* textColour) override;

    // AlertWindow styling
    void drawAlertBox (juce::Graphics& g, juce::AlertWindow& alert,
                       const juce::Rectangle<int>& textArea,
                       juce::TextLayout& textLayout) override;

    juce::Font getAlertWindowTitleFont() override;
    juce::Font getAlertWindowMessageFont() override;
    juce::Font getAlertWindowFont() override;

private:
    // Color scheme constants
    static constexpr juce::uint32 BACKGROUND_DARK = 0xff2d2d30;
    static constexpr juce::uint32 BACKGROUND_DARKER = 0xff1e1e20;
    static constexpr juce::uint32 ACCENT_TEAL = 0xff4dd0e1;
    static constexpr juce::uint32 TEXT_WHITE = 0xffffffff;
    static constexpr juce::uint32 BUTTON_NORMAL = 0xff404040;
    static constexpr juce::uint32 BUTTON_HOVER = 0xff505050;
    static constexpr juce::uint32 BUTTON_DOWN = 0xff606060;

    // Helper methods
    void drawRoundedButton (juce::Graphics& g, const juce::Rectangle<float>& bounds,
                            juce::Colour fillColour, juce::Colour outlineColour,
                            float cornerRadius = 8.0f) const;

    void drawDigitalDisplay (juce::Graphics& g, const juce::Rectangle<float>& bounds,
                             const juce::String& text) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Style)
};

} // namespace retuner
