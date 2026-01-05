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
    # Check for signtool.exe (Windows SDK) - prefer newer versions
    # Microsoft Trusted Signing requires minimum SDK version 10.0.22621.755
    # Clear cache to ensure we get the newest version
    unset(SIGNTOOL_EXECUTABLE CACHE)
    find_program(SIGNTOOL_EXECUTABLE signtool.exe
        HINTS
            "C:/Program Files (x86)/Windows Kits/10/bin/10.0.26100.0/x64"
            "C:/Program Files (x86)/Windows Kits/10/bin/10.0.22621.0/x64"
        NO_DEFAULT_PATH
    )
    
    if(SIGNTOOL_EXECUTABLE)
        message(STATUS "signtool.exe found: ${SIGNTOOL_EXECUTABLE}")
        
        # Store path with forward slashes
        set(SIGNTOOL_PATH "${SIGNTOOL_EXECUTABLE}")
        
        # Artifact paths using generator expressions for configuration
        set(ARTIFACTS_DIR "${PROJECT_BINARY_DIR}/reTuner_artefacts/$<CONFIG>")
        set(VST3_BINARY "${ARTIFACTS_DIR}/VST3/KV-reTuner.vst3/Contents/x86_64-win/KV-reTuner.vst3")
        set(CLAP_FILE "${ARTIFACTS_DIR}/CLAP/KV-reTuner.clap")
        set(LV2_DLL "${ARTIFACTS_DIR}/LV2/KV-reTuner.lv2/KV-reTuner.dll")
        set(STANDALONE_EXE "${PROJECT_BINARY_DIR}/src/app/reTunerApp_artefacts/$<CONFIG>/reTuner.exe")
        
        # Check for Microsoft Trusted Signing configuration
        if(NOT TRUSTED_SIGNING_DLIB AND DEFINED ENV{TRUSTED_SIGNING_DLIB})
            string(STRIP "$ENV{TRUSTED_SIGNING_DLIB}" TRUSTED_SIGNING_DLIB)
        endif()
        
        if(NOT TRUSTED_SIGNING_ACCOUNT AND DEFINED ENV{TRUSTED_SIGNING_ACCOUNT})
            string(STRIP "$ENV{TRUSTED_SIGNING_ACCOUNT}" TRUSTED_SIGNING_ACCOUNT)
        endif()
        
        if(NOT TRUSTED_SIGNING_PROFILE AND DEFINED ENV{TRUSTED_SIGNING_PROFILE})
            string(STRIP "$ENV{TRUSTED_SIGNING_PROFILE}" TRUSTED_SIGNING_PROFILE)
        endif()
        
        if(NOT TRUSTED_SIGNING_TENANT_ID AND DEFINED ENV{TRUSTED_SIGNING_TENANT_ID})
            string(STRIP "$ENV{TRUSTED_SIGNING_TENANT_ID}" TRUSTED_SIGNING_TENANT_ID)
        endif()

        # Build signing command based on available configuration
        if(TRUSTED_SIGNING_DLIB AND TRUSTED_SIGNING_ACCOUNT AND TRUSTED_SIGNING_PROFILE AND TRUSTED_SIGNING_TENANT_ID)
            message(STATUS "Using Microsoft Trusted Signing")
            message(STATUS "  Account: ${TRUSTED_SIGNING_ACCOUNT}")
            message(STATUS "  Profile: ${TRUSTED_SIGNING_PROFILE}")
            message(STATUS "  TenantId: ${TRUSTED_SIGNING_TENANT_ID}")
            
            # Generate metadata JSON file
            set(METADATA_FILE "${PROJECT_BINARY_DIR}/trusted_signing_metadata.json")
            file(WRITE "${METADATA_FILE}"
"{\n\
    \"Endpoint\": \"https://eus.codesigning.azure.net/\",\n\
    \"CodeSigningAccountName\": \"${TRUSTED_SIGNING_ACCOUNT}\",\n\
    \"CertificateProfileName\": \"${TRUSTED_SIGNING_PROFILE}\",\n\
    \"TenantId\": \"${TRUSTED_SIGNING_TENANT_ID}\"\n\
}\n")
            
            # Build common signtool command (add /v /debug if you get problems)
            set(SIGNTOOL_CMD "${SIGNTOOL_PATH}" sign /fd SHA256 
                /tr http://timestamp.acs.microsoft.com 
                /td SHA256 
                /dlib "${TRUSTED_SIGNING_DLIB}" 
                /dmdf "${METADATA_FILE}")
            # set(SIGNTOOL_CMD "${SIGNTOOL_CMD}" PARENT_SCOPE)
            
            add_custom_target(sign-products
                COMMAND ${SIGNTOOL_CMD} "${VST3_BINARY}"
                COMMAND ${SIGNTOOL_CMD} "${CLAP_FILE}"
                COMMAND ${SIGNTOOL_CMD} "${LV2_DLL}"
                COMMAND ${SIGNTOOL_CMD} "${STANDALONE_EXE}"
                
                COMMENT "Code signing all plugin formats and standalone app with Microsoft Trusted Signing"
            )
        else()
            message(STATUS "Microsoft Trusted Signing not configured")
            message(STATUS "Set TRUSTED_SIGNING_DLIB, TRUSTED_SIGNING_ACCOUNT, and TRUSTED_SIGNING_PROFILE to enable")
        endif()
    else()
        message(STATUS "signtool.exe not found - Windows code signing not available")
    endif()
else()
    # noop
endif()
