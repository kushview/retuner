
juce_add_gui_app(reTunerApp
    COMPANY_NAME "Kushview"
    PRODUCT_NAME "reTuner"
    BUNDLE_ID "net.kushview.reTunerApp"
    ICON_BIG "retuner-icon.png"
)

target_sources(reTunerApp PRIVATE
    application.cpp
    application.hpp
    mainwindow.cpp
    mainwindow.hpp
    contentcomponent.cpp
    contentcomponent.hpp
    mediaplayercomponent.cpp
    mediaplayercomponent.hpp
    audioengine.cpp
    audioengine.hpp
    exporter.cpp
    exporter.hpp
    exportdialog.cpp
    exportdialog.hpp
    exportthread.cpp
    exportthread.hpp
    ../processor.cpp
    ../editor.cpp
    ../style.cpp
)

if(MSVC)
    target_compile_options(reTunerApp PRIVATE
        /wd4996  # Disable deprecation warnings (JUCE AudioParameterFloat)
        /wd4244  # Disable conversion warnings (RubberBand Options)
    )
endif()

target_link_libraries(reTunerApp PRIVATE
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    rubberband
)

target_compile_definitions(reTunerApp PRIVATE
    RETUNER_APP=1
    RETUNER_VERSION=\"${PROJECT_VERSION}\"
    ${RETUNER_JUCE_OPTIONS}
)

if(LINUX)
    # Linux conventions want all lowercase executable names
    set_target_properties(reTunerApp PROPERTIES
        OUTPUT_NAME "retuner"
    )
    
    # Configure and install desktop file
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/retuner.desktop.in
        ${CMAKE_CURRENT_BINARY_DIR}/retuner.desktop
        @ONLY
    )
    
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/retuner.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )
    
    # Install icon if available
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/retuner-icon.png)
        install(FILES retuner-icon.png
            DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/512x512/apps
            RENAME retuner.png
        )
    endif()
endif()

if(NOT APPLE)
    install(TARGETS reTunerApp DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
    install(TARGETS reTunerApp
        BUNDLE DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()
