cmake_minimum_required(VERSION 3.21)
project(retuner VERSION 1.1.0)

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
        rubberband
)

if(LINUX OR WIN32)
    install(TARGETS reTuner_CLAP DESTINATION "${CMAKE_INSTALL_LIBDIR}/clap")
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/reTuner_artefacts/$<CONFIG>/LV2/"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/lv2")
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/reTuner_artefacts/$<CONFIG>/VST3/"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/vst3")
elseif(APPLE)
    install(TARGETS "reTuner_AU"   LIBRARY DESTINATION "Plug-Ins/AU")
    install(TARGETS "reTuner_CLAP" LIBRARY DESTINATION "Plug-Ins/CLAP")
    install(TARGETS "reTuner_VST3" LIBRARY DESTINATION "Plug-Ins/VST3")
    # LV2 on macOS isn't a real library bundle.
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/reTuner_artefacts/$<CONFIG>/LV2/"
        DESTINATION "Plug-Ins/LV2")
endif()

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
            "${CMAKE_BINARY_DIR}/manual.html"
    )
    if(LINUX)
        install(FILES "${CMAKE_BINARY_DIR}/manual.html"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/retuner")
    else()
        install(FILES "${CMAKE_BINARY_DIR}/manual.html"
            DESTINATION "${CMAKE_INSTALL_PREFIX}")
    endif()
else()
    message(STATUS "pandoc not found - documentation target will not be available")
endif()

add_subdirectory(src/app)
add_subdirectory(test)

if(LINUX)
    install(FILES LICENSE.txt DESTINATION "${CMAKE_INSTALL_DATADIR}/retuner")
else()
    install(FILES LICENSE.txt DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()

# CPack configuration

set(CPACK_PACKAGE_NAME "reTuner")
set(CPACK_PACKAGE_VENDOR "Kushview")
set(CPACK_PACKAGE_CONTACT "Kushview Support <support@kushview.net>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${RETUNER_DESCRIPTION_SUMMARY}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "reTuner")
set(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/INSTALLER.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE.txt")

if(LINUX)
    set(RETUNER_SYSTEM_NAME "linux")
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" RETUNER_PROCESSOR)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_ARCHIVE_COMPONENT_INSTALL OFF)
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libasound2;libfreetype6;libx11-6;libxext6;libxrandr2;libxinerama1;libxcursor1;libcurl4;libgl1;libstdc++6")
elseif(APPLE)
    set(RETUNER_SYSTEM_NAME "macos")
    set(RETUNER_PROCESSOR "universal")
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_ARCHIVE_COMPONENT_INSTALL OFF)
    set(CPACK_SET_DESTDIR ON)
    set(CPACK_PACKAGING_INSTALL_PREFIX "/")
elseif(WIN32)
    set(RETUNER_SYSTEM_NAME "windows")
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" RETUNER_PROCESSOR)
    set(CPACK_IGNORE_FILES ".*\\.lib$;.*\\.exp$;.*\\.ico$")
    set(CPACK_GENERATOR "ZIP")
    set(CPACK_ARCHIVE_COMPONENT_INSTALL OFF)
    set(CPACK_PACKAGING_INSTALL_PREFIX "/")
endif()

set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${CPACK_PACKAGE_VERSION}-${RETUNER_SYSTEM_NAME}-${RETUNER_PROCESSOR}")
include(CPack)
