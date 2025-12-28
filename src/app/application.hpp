// Copyright (c) 2025 Kushview, LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "audioengine.hpp"
#include "../style.hpp"

namespace retuner {
namespace app {

class Settings : public juce::ApplicationProperties {
public:
    void setup (const juce::String& appName, const juce::String& company = {})
    {
        juce::PropertiesFile::Options opts;
        opts.applicationName = appName;
        opts.filenameSuffix = "settings"; // produces e.g. settings.settings (XML)
        opts.osxLibrarySubFolder = "Application Support";
        opts.folderName = company.isNotEmpty() ? (company + "/" + appName) : appName;
        opts.storageFormat = juce::PropertiesFile::storeAsXML;
        opts.commonToAllUsers = false;
        setStorageParameters (opts);
    }

    // Simple string storage for XML/Base64 blobs
    void setAudioDeviceStateXML (const juce::XmlElement* xml)
    {
        if (auto* f = getUserSettings()) {
            f->setValue ("audioDeviceStateXML", xml);
        }
    }
    std::unique_ptr<juce::XmlElement> audioDeviceStateXML() const
    {
        if (auto* f = const_cast<Settings*> (this)->getUserSettings())
            return f->getXmlValue ("audioDeviceStateXML");
        return nullptr;
    }

    void setProcessorStateBase64 (const juce::String& b64)
    {
        if (auto* f = getUserSettings()) {
            f->setValue ("processorStateB64", b64);
        }
    }
    juce::String processorStateBase64() const
    {
        if (auto* f = const_cast<Settings*> (this)->getUserSettings())
            return f->getValue ("processorStateB64");
        return {};
    }

    void setLastLoadedFile (const juce::String& filePath)
    {
        if (auto* f = getUserSettings()) {
            f->setValue ("lastLoadedFile", filePath);
        }
    }
    juce::String lastLoadedFile() const
    {
        if (auto* f = const_cast<Settings*> (this)->getUserSettings())
            return f->getValue ("lastLoadedFile");
        return {};
    }

    void flush()
    {
        if (auto* f = getUserSettings())
            f->saveIfNeeded();
    }
};

class Application : public juce::JUCEApplication {
public:
    Application() = default;

    const juce::String getApplicationName() override { return "reTuner"; }
    const juce::String getApplicationVersion() override { return RETUNER_VERSION; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise (const juce::String& commandLine) override;
    void shutdown() override;
    void anotherInstanceStarted (const juce::String& commandLine) override;

    void systemRequestedQuit() override;

private:
    std::unique_ptr<juce::DocumentWindow> _mainWindow;
    Settings _settings;
    std::unique_ptr<AudioEngine> _engine;
    std::unique_ptr<Style> _modernLookAndFeel;

public:
    Settings& settings() noexcept { return _settings; }
    static Settings& settingsRef();
    AudioEngine& engine() noexcept
    {
        jassert (_engine);
        return *_engine;
    }
    static AudioEngine& engineRef();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Application)
};

} // namespace app
} // namespace retuner