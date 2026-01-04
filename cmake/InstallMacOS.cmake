if(RETUNER_CPACK)
    set(RETUNER_APPDIR "Applications")
    set(RETUNER_PLUGINDIR "Library/Audio/Plug-Ins")
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
