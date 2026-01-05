if(RETUNER_CPACK AND RETUNER_NSIS)
    # Standalone app - relative to user-chosen INSTDIR
    install(TARGETS reTunerApp DESTINATION "." COMPONENT Standalone)
    install(FILES LICENSE.txt DESTINATION "." COMPONENT Standalone)
    if(PANDOC_EXECUTABLE)
        install(FILES "${PROJECT_BINARY_DIR}/manual.html"
            DESTINATION "." COMPONENT Standalone)
    endif()

    # Plugins - installed to component-specific directories (set in Packaging.cmake)
    install(TARGETS reTuner_CLAP 
        DESTINATION "."
        COMPONENT CLAP)
    install(DIRECTORY "${PROJECT_BINARY_DIR}/reTuner_artefacts/$<CONFIG>/LV2/"
        DESTINATION "."
        COMPONENT LV2
        PATTERN "*.lib" EXCLUDE
        PATTERN "*.exp" EXCLUDE)
    install(DIRECTORY "${PROJECT_BINARY_DIR}/reTuner_artefacts/$<CONFIG>/VST3/"
        DESTINATION "."
        COMPONENT VST3
        PATTERN "*.lib" EXCLUDE
        PATTERN "*.exp" EXCLUDE)
else()
    if(PANDOC_EXECUTABLE)
        install(FILES "${PROJECT_BINARY_DIR}/manual.html"
            DESTINATION ".")
    endif()

    install(FILES LICENSE.txt DESTINATION ".")

    install(TARGETS reTuner_CLAP DESTINATION "CLAP")
    install(DIRECTORY "${PROJECT_BINARY_DIR}/reTuner_artefacts/$<CONFIG>/LV2/"
        DESTINATION "LV2"
        PATTERN "*.lib" EXCLUDE
        PATTERN "*.exp" EXCLUDE)
    install(DIRECTORY "${PROJECT_BINARY_DIR}/reTuner_artefacts/$<CONFIG>/VST3/"
        DESTINATION "VST3"
        PATTERN "*.lib" EXCLUDE
        PATTERN "*.exp" EXCLUDE)

    install(TARGETS reTunerApp DESTINATION ".")
endif()
