// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.hpp"
#include "contentcomponent.hpp"
#include "application.hpp"
#include "exportdialog.hpp"
#include "exportthread.hpp"
#include <juce_audio_utils/juce_audio_utils.h>

namespace retuner {
namespace app {

MainWindow::MainWindow (const juce::String& name)
    : juce::DocumentWindow (name,
                            juce::Colour (0xff2a2a2a), // Dark background
                            juce::DocumentWindow::allButtons)
{
    setUsingNativeTitleBar (true);
    setupMenuBar();
    setSize (500, 400);
    auto* c = new ContentComponent();

    setContentOwned (c, true);

    centreWithSize (getWidth(), getHeight());

    setLookAndFeel();
    setResizable (false, false);
}

MainWindow::~MainWindow()
{
#if JUCE_MAC
    juce::MenuBarModel::setMacMainMenu (nullptr);
#else
    setMenuBar (nullptr);
#endif
}

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

void MainWindow::resized()
{
    juce::DocumentWindow::resized();
}

void MainWindow::setLookAndFeel()
{
    // Apply dark theme styling
    auto& lf = getLookAndFeel();
    lf.setColour (juce::ResizableWindow::backgroundColourId, juce::Colour (0xff2a2a2a));
    lf.setColour (juce::DocumentWindow::textColourId, juce::Colours::white);
}

void MainWindow::setupMenuBar()
{
#if JUCE_MAC
    // On Mac, use the native menu bar
    juce::MenuBarModel::setMacMainMenu (this);
#else
    // On other platforms, use the document window's built-in menu bar
    setMenuBar (this);
#endif
}

juce::StringArray MainWindow::getMenuBarNames()
{
    return { "File", "Help" };
}

juce::PopupMenu MainWindow::getMenuForIndex (int topLevelMenuIndex, const juce::String& menuName)
{
    juce::PopupMenu menu;

    if (menuName == "File") {
        menu.addItem (fileOpen, "Open...", true);
        menu.addSeparator();
        menu.addItem (fileResetProcessorState, "Reset Defaults");
        menu.addItem (fileExport, "Export...", hasAudioFileLoaded());
        menu.addSeparator();
        menu.addItem (filePreferences, "Preferences...", true);
        menu.addSeparator();
        menu.addItem (fileQuit, "Quit", true);
    } else if (menuName == "Help") {
        menu.addItem (helpUserManual, "User Manual", true);
        menu.addSeparator();
        menu.addItem (helpAbout, "About ReTuner", true);
    }

    return menu;
}

void MainWindow::menuItemSelected (int menuItemID, int topLevelMenuIndex)
{
    juce::ignoreUnused (topLevelMenuIndex);

    switch (menuItemID) {
        case fileOpen:
            showOpenFileDialog();
            break;

        case fileExport:
            showExportDialog();
            break;

        case filePreferences:
            openAudioSettingsDialog();
            break;

        case fileQuit:
            closeButtonPressed();
            break;

        case fileResetProcessorState:
            resetProcessorState();
            break;

        case helpUserManual:
            // TODO: Open user manual or help documentation
            juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::InfoIcon,
                                                    "User Manual",
                                                    "User manual coming soon!\n\nFor now, ReTuner is a frequency retuning tool that converts audio from one reference frequency to another.");
            break;

        case helpAbout:
            juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::InfoIcon,
                                                    "About ReTuner",
                                                    "ReTuner v1.0\n\nA professional audio frequency retuning tool.\n\nBuilt with JUCE framework.\n\n(c) 2025 Kushview");
            break;

        default:
            break;
    }
}

void MainWindow::openAudioSettingsDialog()
{
    // Get the audio engine from the application
    auto& engine = Application::engineRef();
    auto& dm = engine.deviceManager();

    // Create audio settings component
    auto settings = std::make_unique<juce::AudioDeviceSelectorComponent> (
        dm,
        /* minInputChannels */ 0,
        /* maxInputChannels */ 2,
        /* minOutputChannels */ 1,
        /* maxOutputChannels */ 2,
        /* showMidiInputOptions */ false,
        /* showMidiOutputSelector */ false,
        /* treatAsIODevice */ false,
        /* hideAdvancedOptionsWithButton */ false);

    settings->setSize (500, 400);

    juce::DialogWindow::LaunchOptions opts;
    opts.dialogTitle = "Audio Device Settings";
    opts.componentToCentreAround = this;
    opts.content.setOwned (settings.release());
    opts.dialogBackgroundColour = juce::Colour (0xff2a2a2a);
    opts.escapeKeyTriggersCloseButton = true;
    opts.useNativeTitleBar = true;
    opts.resizable = true;

    opts.launchAsync();
}

bool MainWindow::hasAudioFileLoaded() const
{
    return Application::engineRef().hasFileLoaded();
}

void MainWindow::showExportDialog()
{
    auto& engine = Application::engineRef();

    if (! engine.hasFileLoaded()) {
        juce::AlertWindow::showMessageBoxAsync (
            juce::MessageBoxIconType::WarningIcon,
            "No File Loaded",
            "Please load an audio file first before exporting.");
        return;
    }

    // Get current frequencies from processor
    auto* processor = engine.processor();
    if (processor == nullptr) {
        juce::AlertWindow::showMessageBoxAsync (
            juce::MessageBoxIconType::WarningIcon,
            "Export Error",
            "ReTuner processor not available.");
        return;
    }

    float sourceFreq = engine.sourceFrequency();
    float targetFreq = engine.targetFrequency();

    // Create export dialog
    auto dialog = std::make_unique<ExportDialog> (engine.currentFile());
    auto* dialogPtr = dialog.get();

    // Set up button callbacks
    dialogPtr->onExport = [dialogPtr, sourceFreq, targetFreq]() {
        auto outputFile = dialogPtr->outputFile();

        if (outputFile == juce::File()) {
            juce::AlertWindow::showMessageBoxAsync (
                juce::MessageBoxIconType::WarningIcon,
                "Invalid Output",
                "Please select a valid output file.");
            return;
        }

        // Create export settings
        Exporter::ExportSettings settings;
        settings.quality = dialogPtr->quality();
        settings.format = dialogPtr->format();
        settings.bitDepth = dialogPtr->bitDepth();
        settings.enableUpsampling = dialogPtr->shouldUpsample();

        if (settings.enableUpsampling)
            settings.upsampleRate = 96000.0;

        // Get input file from engine
        auto inputFile = Application::engineRef().currentFile();

        // Create and launch export thread
        // Thread will delete itself when complete (see threadComplete())
        auto* exportThread = new ExportThread (inputFile, outputFile, settings, sourceFreq, targetFreq);
        exportThread->launchThread();

        // Close the dialog
        if (auto* window = dialogPtr->findParentComponentOfClass<juce::DialogWindow>())
            window->exitModalState (1);
    };

    dialogPtr->onCancel = [dialogPtr]() {
        if (auto* window = dialogPtr->findParentComponentOfClass<juce::DialogWindow>())
            window->exitModalState (0);
    };

    // Launch dialog
    juce::DialogWindow::LaunchOptions opts;
    opts.dialogTitle = "Export Audio";
    opts.componentToCentreAround = this;
    opts.content.setOwned (dialog.release());
    opts.dialogBackgroundColour = juce::Colour (0xff2a2a2a);
    opts.escapeKeyTriggersCloseButton = true;
    opts.useNativeTitleBar = true;
    opts.resizable = false;

    opts.launchAsync();
}

void MainWindow::resetProcessorState()
{
    auto& engine = Application::engineRef();
    auto* processor = engine.processor();

    if (processor == nullptr) {
        juce::AlertWindow::showMessageBoxAsync (
            juce::MessageBoxIconType::WarningIcon,
            "Reset Failed",
            "ReTuner processor not available.");
        return;
    }

    // Reset processor to default state
    for (auto* param : processor->getParameters()) {
        if (param->getName (100).toLowerCase().contains ("volume"))
            continue;
        param->setValueNotifyingHost (param->getDefaultValue());
    }

    // Clear saved state from settings
    auto& settings = Application::settingsRef();
    settings.setProcessorStateBase64 (juce::String());
    settings.flush();
}

void MainWindow::showOpenFileDialog()
{
    auto& engine = Application::engineRef();
    auto& formats = engine.formatManager();

    // Create file chooser (stored as member to keep it alive during async operation)
    static std::unique_ptr<juce::FileChooser> fileChooser;

    fileChooser = std::make_unique<juce::FileChooser> (
        "Select an audio file to play...",
        juce::File(),
        formats.getWildcardForAllFormats());

    auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync (flags, [this] (const juce::FileChooser& chooser) {
        auto file = chooser.getResult();
        if (file.existsAsFile()) {
            if (Application::engineRef().loadAudioFile (file)) {
                // File loaded successfully - UI will update automatically via engine callbacks
            } else {
                juce::AlertWindow::showMessageBoxAsync (
                    juce::MessageBoxIconType::WarningIcon,
                    "Load Error",
                    "Failed to load the selected audio file.");
            }
        }
    });
}

} // namespace app
} // namespace retuner
