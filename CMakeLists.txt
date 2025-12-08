
cmake_minimum_required(VERSION 3.21)
project(retuner VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(OSX_ARCHITECTURES arm64 x86_64)

enable_testing()

## Rubberband
add_library(rubberband STATIC deps/rubberband/single/RubberBandSingle.cpp)
target_include_directories(rubberband PUBLIC deps/rubberband)

if(WIN32)
    target_compile_definitions(rubberband PRIVATE _WIN32)
else()
    target_compile_definitions(rubberband PRIVATE USE_PTHREADS)
endif()

find_package(Threads REQUIRED)
target_link_libraries(rubberband PUBLIC Threads::Threads)
if(UNIX)
    target_link_libraries(rubberband PUBLIC m)
endif()

## JUCE
add_subdirectory(deps/JUCE EXCLUDE_FROM_ALL)
set_directory_properties(PROPERTIES JUCE_COMPANY_NAME Kushview)
set_directory_properties(PROPERTIES JUCE_COMPANY_COPYRIGHT "Copyright (c) 2025 Kushview, LLC")
set_directory_properties(PROPERTIES JUCE_COMPANY_WEBSITE "https://kushview.net")
set_directory_properties(PROPERTIES JUCE_COMPANY_EMAIL "support@kushview.net")

## CLAP
set(CMAKE_POLICY_DEFAULT_CMP0177 OLD)
add_subdirectory(deps/clap-juce-extensions EXCLUDE_FROM_ALL)

## reTuner Audio Plugins
juce_add_plugin(reTuner
    PRODUCT_NAME "KV-reTuner"
    PLUGIN_NAME "reTuner"
    FORMATS AU VST3 LV2
    PLUGIN_MANUFACTURER_CODE KshV
    PLUGIN_CODE Retn

    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    
    BUNDLE_ID "net.kushview.plugins.reTuner"
    LV2URI "https://kushview.net/plugins/retuner"
    VST3_CATEGORIES Fx "Pitch Shift"
    AU_MAIN_TYPE "kAudioUnitType_Effect"

    HARDENED_RUNTIME_ENABLED TRUE
    HARDENED_RUNTIME_OPTIONS
        "com.apple.security.cs.allow-jit"
	    "com.apple.security.cs.disable-library-validation"
	    "com.apple.security.device.audio-input"
)
clap_juce_extensions_plugin(TARGET reTuner
    CLAP_ID "net.kushview.plugins.reTuner"
    CLAP_FEATURES audio-effect pitch-shifter)

set(RETUNER_JUCE_OPTIONS
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=1
    JUCE_LOAD_CURL_SYMBOLS_LAZILY=1
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_USE_LAME_AUDIO_FORMAT=0
    JUCE_USE_MP3AUDIOFORMAT=1
)

target_sources(reTuner PRIVATE
    src/processor.cpp
    src/processor.hpp
    src/editor.cpp
    src/editor.hpp
    src/style.cpp
    src/style.hpp
)

target_compile_definitions(reTuner PRIVATE
    ${RETUNER_JUCE_OPTIONS}
)

target_link_libraries(reTuner 
    PRIVATE
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        rubberband
)

add_subdirectory(src/app)
add_subdirectory(test)
