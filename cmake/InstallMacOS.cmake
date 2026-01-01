if(RETUNER_CPACK)
    if(RETUNER_DMG)
        set(RETUNER_APPDIR "${CMAKE_INSTALL_PREFIX}")
        set(RETUNER_PLUGINDIR "Plug-Ins")
        if(PANDOC_EXECUTABLE)
            install(FILES "${PROJECT_BINARY_DIR}/manual.html" DESTINATION "${CMAKE_INSTALL_PREFIX}")
        endif()
        install(FILES "${PROJECT_SOURCE_DIR}/LICENSE.txt" DESTINATION "${CMAKE_INSTALL_PREFIX}")
    else()
        set(RETUNER_APPDIR "Applications")
        set(RETUNER_PLUGINDIR "Library/Audio/Plug-Ins")
    endif()
else()
    set(RETUNER_APPDIR "Applications")
    set(RETUNER_PLUGINDIR "Library/Audio/Plug-Ins")
endif()

install(TARGETS reTuner_AU   LIBRARY 
    DESTINATION "${RETUNER_PLUGINDIR}/Components"
    COMPONENT AU)
install(TARGETS reTuner_CLAP LIBRARY 
    DESTINATION "${RETUNER_PLUGINDIR}/CLAP"
    COMPONENT CLAP)
install(TARGETS reTuner_VST3 LIBRARY 
    DESTINATION "${RETUNER_PLUGINDIR}/VST3"
    COMPONENT VST3)
# LV2 on macOS isn't a real library bundle.
install(DIRECTORY "${PROJECT_BINARY_DIR}/reTuner_artefacts/$<CONFIG>/LV2/"
    DESTINATION "${RETUNER_PLUGINDIR}/LV2"
    COMPONENT LV2)

install(TARGETS reTunerApp 
    BUNDLE DESTINATION "${RETUNER_APPDIR}" 
    COMPONENT Standalone)
