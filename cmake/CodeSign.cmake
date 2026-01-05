if(APPLE)   
    # Code signing identity - check CMake variable, then environment, default to ad-hoc
    if(NOT CODE_SIGN_IDENTITY)
        if(DEFINED ENV{CODE_SIGN_IDENTITY})
            set(CODE_SIGN_IDENTITY "$ENV{CODE_SIGN_IDENTITY}")
        else()
            set(CODE_SIGN_IDENTITY "-")
            message(STATUS "No CODE_SIGN_IDENTITY set, using ad-hoc signing")
        endif()
    endif()
    
    message(STATUS "Code signing identity: ${CODE_SIGN_IDENTITY}")
    
    # Entitlements file
    set(ENTITLEMENTS_FILE "${PROJECT_SOURCE_DIR}/plugin.entitlements")
    
    # Artifact paths using generator expressions for configuration
    set(ARTIFACTS_DIR "${PROJECT_BINARY_DIR}/reTuner_artefacts/$<CONFIG>")
    set(AU_BUNDLE "${ARTIFACTS_DIR}/AU/KV-reTuner.component")
    set(VST3_BUNDLE "${ARTIFACTS_DIR}/VST3/KV-reTuner.vst3")
    set(CLAP_BUNDLE "${ARTIFACTS_DIR}/CLAP/KV-reTuner.clap")
    set(LV2_SO "${ARTIFACTS_DIR}/LV2/KV-reTuner.lv2/libKV-reTuner.so")
    set(STANDALONE_APP "${PROJECT_BINARY_DIR}/src/app/reTunerApp_artefacts/$<CONFIG>/reTuner.app")
    
    add_custom_target(sign-products  
        COMMAND echo "Signing AU bundle..."
        COMMAND codesign --force --deep --sign "${CODE_SIGN_IDENTITY}" 
            --timestamp --options runtime --entitlements "${ENTITLEMENTS_FILE}" "${AU_BUNDLE}"
        COMMAND codesign --verify --deep --strict "${AU_BUNDLE}"
        COMMAND echo "✓ Valid signature: AU bundle"
        
        COMMAND echo "Signing VST3 bundle..."
        COMMAND codesign --force --deep --sign "${CODE_SIGN_IDENTITY}" 
            --timestamp --options runtime --entitlements "${ENTITLEMENTS_FILE}" "${VST3_BUNDLE}"
        COMMAND codesign --verify --deep --strict "${VST3_BUNDLE}"
        COMMAND echo "✓ Valid signature: VST3 bundle"
        
        COMMAND echo "Signing CLAP bundle..."
        COMMAND codesign --force --deep --sign "${CODE_SIGN_IDENTITY}" 
            --timestamp --options runtime --entitlements "${ENTITLEMENTS_FILE}" "${CLAP_BUNDLE}"
        COMMAND codesign --verify --deep --strict "${CLAP_BUNDLE}"
        COMMAND echo "✓ Valid signature: CLAP bundle"
        
        COMMAND echo "Signing LV2 shared library..."
        COMMAND codesign --force --sign "${CODE_SIGN_IDENTITY}" 
            --timestamp --deep --options runtime --entitlements "${ENTITLEMENTS_FILE}" "${LV2_SO}"
        COMMAND codesign --verify --strict "${LV2_SO}"
        COMMAND echo "✓ Valid signature: LV2 shared library"
        
        COMMAND echo "Signing standalone app..."
        COMMAND codesign --force --deep --sign "${CODE_SIGN_IDENTITY}" 
            --timestamp --options runtime --entitlements "${ENTITLEMENTS_FILE}" "${STANDALONE_APP}"
        COMMAND codesign --verify --deep --strict "${STANDALONE_APP}"
        COMMAND echo "✓ Valid signature: Standalone app"
        
        COMMENT "Code signing all plugin formats and standalone app"
        VERBATIM
    )
elseif(WIN32)

else()
    # noop
endif()
