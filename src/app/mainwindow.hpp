// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace retuner {
namespace app {

class MainWindow : public juce::DocumentWindow,
                   public juce::MenuBarModel {
public:
    explicit MainWindow (const juce::String& name);
    ~MainWindow() override;

    void closeButtonPressed() override;
    void resized() override;

    // MenuBarModel interface
    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex (int topLevelMenuIndex, const juce::String& menuName) override;
    void menuItemSelected (int menuItemID, int topLevelMenuIndex) override;

private:
    void setLookAndFeel();

private:
    void setupMenuBar();
    void openAudioSettingsDialog();
    void showExportDialog();
    void showOpenFileDialog();
    void resetProcessorState();
    bool hasAudioFileLoaded() const;

    enum MenuItemIDs {
        fileOpen = 1001,
        fileExport = 1003,
        fileResetProcessorState = 1004,
        filePreferences = 1006,
        fileQuit = 1007,

        helpAbout = 4000,
        helpUserManual
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
};

} // namespace app
} // namespace retuner