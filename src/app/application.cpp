// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#include "application.hpp"
#include "mainwindow.hpp"

namespace retuner {
namespace app {

void Application::initialise (const juce::String& commandLine)
{
    juce::ignoreUnused (commandLine);
    // Setup persistent settings storage
    _settings.setup (getApplicationName());

    // Set up global look and feel before creating any components
    _modernLookAndFeel = std::make_unique<Style>();
    juce::LookAndFeel::setDefaultLookAndFeel (_modernLookAndFeel.get());

    // Create the shared audio engine and initialize it
    _engine = std::make_unique<AudioEngine>();
    _engine->initialize();

    // Restore last loaded file after engine is initialized
    _engine->restoreLastLoadedFile();

    _mainWindow = std::make_unique<MainWindow> (getApplicationName());
    _mainWindow->setVisible (true);
}

void Application::shutdown()
{
    // Save settings on shutdown
    _mainWindow.reset();
    _engine->shutdown();
    _engine.reset();

    // Clean up global look and feel
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
    _modernLookAndFeel.reset();

    _settings.flush();
}

void Application::anotherInstanceStarted (const juce::String& commandLine)
{
    juce::ignoreUnused (commandLine);
}

void Application::systemRequestedQuit()
{
    quit();
}

Settings& Application::settingsRef()
{
    auto* app = dynamic_cast<Application*> (juce::JUCEApplication::getInstance());
    jassert (app != nullptr);
    return app->_settings;
}

AudioEngine& Application::engineRef()
{
    auto* app = dynamic_cast<Application*> (juce::JUCEApplication::getInstance());
    jassert (app != nullptr && app->_engine);
    return *app->_engine;
}

} // namespace app
} // namespace retuner

// This macro generates the main() function for the application
START_JUCE_APPLICATION (retuner::app::Application)