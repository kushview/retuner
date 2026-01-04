set(CPACK_PACKAGE_NAME "reTuner")
set(CPACK_PACKAGE_VENDOR "Kushview")
set(CPACK_PACKAGE_CONTACT "Kushview Support <support@kushview.net>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "reTuner - Music retuning application and plugins")
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
    set(CPACK_GENERATOR "productbuild")
    set(CPACK_COMPONENT_INSTALL ON)
    set(CPACK_SET_DESTDIR OFF)
    set(CPACK_PACKAGING_INSTALL_PREFIX "/")
    set(CPACK_PRODUCTBUILD_IDENTIFIER "net.kushview.retuner.installer")
elseif(WIN32)
    set(RETUNER_SYSTEM_NAME "windows")
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" RETUNER_PROCESSOR)
    set(CPACK_GENERATOR "ZIP")
    set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
    set(CPACK_PACKAGING_INSTALL_PREFIX "/")
endif()

set(RETUNER_GENERATOR ${CPACK_GENERATOR})
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${CPACK_PACKAGE_VERSION}-${RETUNER_SYSTEM_NAME}-${RETUNER_PROCESSOR}")
include(CPack)

add_custom_target(installer
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${PROJECT_BINARY_DIR}/_CPack_Packages"
    COMMAND ${CMAKE_COMMAND} --build "${PROJECT_BINARY_DIR}" --target package
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${PROJECT_BINARY_DIR}/_CPack_Packages"
    COMMENT "Cleaning CPack staging and building package"
    VERBATIM
    USES_TERMINAL)

if("productbuild" IN_LIST RETUNER_GENERATOR OR RETUNER_GENERATOR STREQUAL "productbuild")
    cpack_add_component(AU
        DISPLAY_NAME "Audio Unit Plugin"
        DESCRIPTION "Audio Unit (AU) plugin format"
        OPTIONAL)
    cpack_add_component(VST3
        DISPLAY_NAME "VST3 Plugin"
        DESCRIPTION "VST3 plugin format"
        OPTIONAL)
    cpack_add_component(CLAP
        DISPLAY_NAME "CLAP Plugin"
        DESCRIPTION "CLAP plugin format"
        OPTIONAL)
    cpack_add_component(LV2
        DISPLAY_NAME "LV2 Plugin"
        DESCRIPTION "LV2 plugin format"
        OPTIONAL)
    cpack_add_component(Standalone
        DISPLAY_NAME "Standalone"
        DESCRIPTION "Standalone application"
        OPTIONAL)
endif()

# Signing and notarization targets for macOS
if(APPLE)
    set(NOTARIZE_FILE "${PROJECT_BINARY_DIR}/${CPACK_PACKAGE_FILE_NAME}.pkg")
    set(UNSIGNED_PKG "${PROJECT_BINARY_DIR}/${CPACK_PACKAGE_FILE_NAME}-unsigned.pkg")

    # Sign the installer package (uses productsign)
    add_custom_target(sign-installer
        COMMAND echo "Signing installer package..."
        COMMAND ${CMAKE_COMMAND} -E rename "${NOTARIZE_FILE}" "${UNSIGNED_PKG}"
        COMMAND productsign --sign "$ENV{INSTALLER_SIGN_IDENTITY}" "${UNSIGNED_PKG}" "${NOTARIZE_FILE}"
        COMMAND ${CMAKE_COMMAND} -E rm -f "${UNSIGNED_PKG}"
        COMMAND echo "✓ Signed: ${NOTARIZE_FILE}"
        COMMENT "Signing productbuild package with productsign"
        VERBATIM
        USES_TERMINAL)

    add_custom_target(notarize
        COMMAND echo "Submitting for notarization: ${NOTARIZE_FILE}"
        COMMAND xcrun notarytool submit "${NOTARIZE_FILE}"
            --apple-id "$ENV{APPLE_ID}"
            --team-id "$ENV{TEAM_ID}"
            --password "$ENV{APP_PASSWORD}"
            --wait
        COMMAND echo "Stapling notarization ticket..."
        COMMAND xcrun stapler staple "${NOTARIZE_FILE}"
        COMMAND echo "✓ Notarized successfully: ${NOTARIZE_FILE}"
        COMMENT "Notarizing installer package"
        VERBATIM
        USES_TERMINAL)

    add_dependencies(notarize sign-products installer)
    add_dependencies(notarize sign-installer)
endif()
