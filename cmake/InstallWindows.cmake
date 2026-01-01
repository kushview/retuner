if(LINUX)
    install(FILES "${CMAKE_BINARY_DIR}/manual.html"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/retuner")
else()
    install(FILES "${CMAKE_BINARY_DIR}/manual.html"
        DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()


if(LINUX)
    install(FILES LICENSE.txt DESTINATION "${CMAKE_INSTALL_DATADIR}/retuner")
else()
    install(FILES LICENSE.txt DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()

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


if(NOT APPLE)
    install(TARGETS reTunerApp DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
    install(TARGETS reTunerApp
        BUNDLE DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()

