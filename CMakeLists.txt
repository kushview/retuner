
cmake_minimum_required(VERSION 3.21)
project(retuner VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(OSX_ARCHITECTURES arm64 x86_64)

# Always add RubberBand as a subproject
# Use the single-file build for simplicity
add_library(rubberband STATIC
    deps/rubberband/single/RubberBandSingle.cpp
)

target_include_directories(rubberband PUBLIC
    deps/rubberband
)

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

set(RUBBERBAND_FOUND TRUE)


if(PROJECT_IS_TOP_LEVEL)
    add_subdirectory(deps/JUCE EXCLUDE_FROM_ALL)
    add_subdirectory(deps/clap-juce-extensions EXCLUDE_FROM_ALL)
endif()

set(RETUNER_JUCE_OPTIONS
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=1
    JUCE_LOAD_CURL_SYMBOLS_LAZILY=1
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_USE_LAME_AUDIO_FORMAT=0
    JUCE_USE_MP3AUDIOFORMAT=1
)

juce_add_plugin(reTuner
    COMPANY_NAME "Kushview"
    COMPANY_WEBSITE "https://kushview.net"
    COMPANY_EMAIL "support@kushview.net"

    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    PLUGIN_MANUFACTURER_CODE KshV
    PLUGIN_CODE Retn
    FORMATS AU VST3 LV2
    PRODUCT_NAME "KV-reTuner"
    PLUGIN_NAME "reTuner"
    BUNDLE_ID "net.kushview.reTuner"
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
    CLAP_ID "net.kushview.reTuner"
    CLAP_FEATURES audio-effect pitch-shifter)

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

target_link_libraries(reTuner PRIVATE
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    rubberband
)

add_subdirectory(src/app)
add_subdirectory(test)
