cmake_minimum_required(VERSION 3.21)
project(retuner VERSION 1.1.0)

option(RETUNER_CPACK "Setup the build for CPack" OFF)

set(RETUNER_DESCRIPTION_SUMMARY, "A music retuning application and plugins")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_OSX_ARCHITECTURES arm64;x86_64)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13")

# Enable ccache if available
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "Found ccache: ${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
else()
    message(STATUS "ccache not found")
endif()

include(GNUInstallDirs)

enable_testing()

## Rubberband
add_library(rubberband STATIC deps/rubberband/single/RubberBandSingle.cpp)
target_include_directories(rubberband PUBLIC deps/rubberband)

if(WIN32)
    target_compile_definitions(rubberband PRIVATE _WIN32 NOMINMAX)

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
set_directory_properties(PROPERTIES JUCE_COMPANY_COPYRIGHT "Copyright (c) Kushview, LLC")
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

if(MSVC)
    target_compile_options(reTuner PRIVATE
        /wd4996  # Disable deprecation warnings (JUCE AudioParameterFloat)
        /wd4244  # Disable conversion warnings (RubberBand Options)
    )
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(reTuner PRIVATE
        -Wno-multichar  # Ignore multi-character character constant warning
    )
endif()

target_link_libraries(reTuner 
    PRIVATE
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        rubberband)

find_program(PANDOC_EXECUTABLE pandoc)
if(PANDOC_EXECUTABLE)
    message(STATUS "Found pandoc: ${PANDOC_EXECUTABLE}")
    
    add_custom_target(reTunerManual ALL
        COMMAND ${PANDOC_EXECUTABLE}
            -o "${CMAKE_BINARY_DIR}/manual.html"
            --standalone
            "${CMAKE_SOURCE_DIR}/docs/manual.md"
        COMMENT "Generating the manual with pandoc"
        VERBATIM
        BYPRODUCTS
            "${CMAKE_BINARY_DIR}/manual.html")
else()
    message(STATUS "pandoc not found - documentation target will not be available")
endif()

add_subdirectory(src/app)
add_subdirectory(test)

# Installation
if(LINUX)
    include(cmake/InstallUnix.cmake)
elseif(APPLE)
    include(cmake/InstallMacOS.cmake)
elseif(WIN32)
    include(cmake/InstallWindows.cmake)
endif()

# Code Signing
include(cmake/CodeSign.cmake)

# CPack configuration
if(RETUNER_CPACK)
    include(cmake/Packaging.cmake)
endif()
