; reTuner Installer
; NSIS installer script for audio plugins

!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "x64.nsh"

; General Information
!define PRODUCT_NAME "Tone Generator"
!define PRODUCT_VERSION "1.0.0"
!define PRODUCT_PUBLISHER "Kushview"
!define PRODUCT_WEB_SITE "https://kushview.net"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; Installer properties
Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "tonegenerator-win64-${PRODUCT_VERSION}-setup.exe"
RequestExecutionLevel admin
SetCompressor lzma

; Version Information
VIProductVersion "1.0.0.0"
VIAddVersionKey "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey "CompanyName" "${PRODUCT_PUBLISHER}"
VIAddVersionKey "LegalCopyright" "© ${PRODUCT_PUBLISHER}"
VIAddVersionKey "FileDescription" "${PRODUCT_NAME} Installer"
VIAddVersionKey "FileVersion" "${PRODUCT_VERSION}"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Default installation directory
InstallDir "$PROGRAMFILES64\${PRODUCT_PUBLISHER}\${PRODUCT_NAME}"

; Get install directory from registry
InstallDirRegKey HKLM "${PRODUCT_UNINST_KEY}" "InstallLocation"

; Variables for custom installation paths
Var VST3_DIR
Var CLAP_DIR
Var LV2_DIR
Var STANDALONE_DIR

; Interface Settings
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\nsis3-metro.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\nsis3-metro.bmp"

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE.txt"
!insertmacro MUI_PAGE_COMPONENTS
Page custom CustomDirectoriesPage CustomDirectoriesPageLeave
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Languages
!insertmacro MUI_LANGUAGE "English"

; Reserve Files
!insertmacro MUI_RESERVEFILE_LANGDLL

; Component Sections
Section "VST3 Plugin" SEC01
;   SectionIn RO
  
  ; Create VST3 directory if it doesn't exist
  CreateDirectory "$VST3_DIR"
  
  ; Copy VST3 plugin
  SetOutPath "$VST3_DIR"
  File /r "..\build\reTuner_artefacts\Release\VST3\KV-reTuner.vst3"
  
  ; Store installation info
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "VST3_DIR" "$VST3_DIR"
SectionEnd

Section "CLAP Plugin" SEC02
  
  ; Create CLAP directory if it doesn't exist
  CreateDirectory "$CLAP_DIR"
  
  ; Copy CLAP plugin
  SetOutPath "$CLAP_DIR"
  File "..\build\reTuner_artefacts\Release\CLAP\KV-reTuner.clap"
  
  ; Store installation info
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "CLAP_DIR" "$CLAP_DIR"
SectionEnd

Section "LV2 Plugin" SEC03
  
  ; Create LV2 directory if it doesn't exist
  CreateDirectory "$LV2_DIR"
  
  ; Copy LV2 plugin
  SetOutPath "$LV2_DIR"
  File /r "..\build\reTuner_artefacts\Release\LV2\KV-reTuner.lv2"
  
  ; Store installation info
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "LV2_DIR" "$LV2_DIR"
SectionEnd

Section "Standalone Application" SEC04
  
  ; Create standalone directory if it doesn't exist
  CreateDirectory "$STANDALONE_DIR"
  
  ; Copy standalone application
  SetOutPath "$STANDALONE_DIR"
  File "..\build\reTuner_artefacts\Release\Standalone\KV-reTuner.exe"
  
  ; Create start menu shortcuts
  CreateDirectory "$SMPROGRAMS\${PRODUCT_PUBLISHER}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_PUBLISHER}\${PRODUCT_NAME}.lnk" "$STANDALONE_DIR\KV-reTuner.exe"
  CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$STANDALONE_DIR\KV-reTuner.exe"
  
  ; Store installation info
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "STANDALONE_DIR" "$STANDALONE_DIR"
SectionEnd

Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateDirectory "$SMPROGRAMS\${PRODUCT_PUBLISHER}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_PUBLISHER}\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_PUBLISHER}\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "InstallLocation" "$INSTDIR"
SectionEnd

; Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} "VST3 plugin for Digital Audio Workstations"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC02} "CLAP plugin for supported Digital Audio Workstations"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC03} "LV2 plugin for supported Digital Audio Workstations"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC04} "Standalone application that can run independently"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; Custom page for directory selection
Function CustomDirectoriesPage
  !insertmacro MUI_HEADER_TEXT "Plugin Installation Directories" "Choose where to install each plugin format."
  
  nsDialogs::Create 1018
  Pop $0
  
  ${If} $0 == error
    Abort
  ${EndIf}
  
  ; VST3 Directory
  ${NSD_CreateLabel} 0 10u 100% 12u "VST3 Path:"
  Pop $0
  
  ${NSD_CreateText} 0 22u 260u 12u "$VST3_DIR"
  Pop $1
  
  ${NSD_CreateButton} 265u 21u 50u 14u "Browse..."
  Pop $2
  ${NSD_OnClick} $2 BrowseVST3
  
  ; CLAP Directory
  ${NSD_CreateLabel} 0 45u 100% 12u "CLAP Path:"
  Pop $0
  
  ${NSD_CreateText} 0 57u 260u 12u "$CLAP_DIR"
  Pop $3
  
  ${NSD_CreateButton} 265u 56u 50u 14u "Browse..."
  Pop $4
  ${NSD_OnClick} $4 BrowseCLAP
  
  ; LV2 Directory
  ${NSD_CreateLabel} 0 80u 100% 12u "LV2 Path:"
  Pop $0
  
  ${NSD_CreateText} 0 92u 260u 12u "$LV2_DIR"
  Pop $5
  
  ${NSD_CreateButton} 265u 91u 50u 14u "Browse..."
  Pop $6
  ${NSD_OnClick} $6 BrowseLV2
  
  ; Standalone Directory
  ${NSD_CreateLabel} 0 115u 100% 12u "Standalone Directory:"
  Pop $0
  
  ${NSD_CreateText} 0 127u 260u 12u "$STANDALONE_DIR"
  Pop $7
  
  ${NSD_CreateButton} 265u 126u 50u 14u "Browse..."
  Pop $8
  ${NSD_OnClick} $8 BrowseStandalone
  
  nsDialogs::Show
FunctionEnd

Function CustomDirectoriesPageLeave
  ${NSD_GetText} $1 $VST3_DIR
  ${NSD_GetText} $3 $CLAP_DIR
  ${NSD_GetText} $5 $LV2_DIR
  ${NSD_GetText} $7 $STANDALONE_DIR
FunctionEnd

Function BrowseVST3
  nsDialogs::SelectFolderDialog "Select VST3 Installation Directory" "$VST3_DIR"
  Pop $0
  ${If} $0 != error
    StrCpy $VST3_DIR "$0"
    ${NSD_SetText} $1 "$VST3_DIR"
  ${EndIf}
FunctionEnd

Function BrowseCLAP
  nsDialogs::SelectFolderDialog "Select CLAP Installation Directory" "$CLAP_DIR"
  Pop $0
  ${If} $0 != error
    StrCpy $CLAP_DIR "$0"
    ${NSD_SetText} $3 "$CLAP_DIR"
  ${EndIf}
FunctionEnd

Function BrowseLV2
  nsDialogs::SelectFolderDialog "Select LV2 Installation Directory" "$LV2_DIR"
  Pop $0
  ${If} $0 != error
    StrCpy $LV2_DIR "$0"
    ${NSD_SetText} $5 "$LV2_DIR"
  ${EndIf}
FunctionEnd

Function BrowseStandalone
  nsDialogs::SelectFolderDialog "Select Standalone Installation Directory" "$STANDALONE_DIR"
  Pop $0
  ${If} $0 != error
    StrCpy $STANDALONE_DIR "$0"
    ${NSD_SetText} $7 "$STANDALONE_DIR"
  ${EndIf}
FunctionEnd

Function .onInit
  ; Set default directories based on common plugin paths
  ${If} ${RunningX64}
    StrCpy $VST3_DIR "$PROGRAMFILES64\Common Files\VST3"
    StrCpy $CLAP_DIR "$PROGRAMFILES64\Common Files\CLAP"
    StrCpy $LV2_DIR "$PROGRAMFILES64\Common Files\LV2"
  ${Else}
    StrCpy $VST3_DIR "$PROGRAMFILES\Common Files\VST3"
    StrCpy $CLAP_DIR "$PROGRAMFILES\Common Files\CLAP"
    StrCpy $LV2_DIR "$PROGRAMFILES\Common Files\LV2"
  ${EndIf}
  
  StrCpy $STANDALONE_DIR "$INSTDIR"
FunctionEnd

; Uninstaller
Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  ; Remove VST3
  ReadRegStr $0 HKLM "${PRODUCT_UNINST_KEY}" "VST3_DIR"
  ${If} $0 != ""
    RMDir /r "$0\KV-reTuner.vst3"
  ${EndIf}
  
  ; Remove CLAP
  ReadRegStr $0 HKLM "${PRODUCT_UNINST_KEY}" "CLAP_DIR"
  ${If} $0 != ""
    Delete "$0\KV-reTuner.clap"
  ${EndIf}
  
  ; Remove LV2
  ReadRegStr $0 HKLM "${PRODUCT_UNINST_KEY}" "LV2_DIR"
  ${If} $0 != ""
    RMDir /r "$0\KV-reTuner.lv2"
  ${EndIf}
  
  ; Remove Standalone
  ReadRegStr $0 HKLM "${PRODUCT_UNINST_KEY}" "STANDALONE_DIR"
  ${If} $0 != ""
    Delete "$0\KV-reTuner.exe"
  ${EndIf}
  
  ; Remove shortcuts
  Delete "$SMPROGRAMS\${PRODUCT_PUBLISHER}\${PRODUCT_NAME}.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_PUBLISHER}\Website.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_PUBLISHER}\Uninstall.lnk"
  Delete "$DESKTOP\${PRODUCT_NAME}.lnk"
  
  RMDir "$SMPROGRAMS\${PRODUCT_PUBLISHER}"
  
  ; Remove main installation directory
  Delete "$INSTDIR\${PRODUCT_NAME}.url"
  Delete "$INSTDIR\uninst.exe"
  RMDir "$INSTDIR"
  
  ; Remove registry entries
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  
  SetAutoClose true
SectionEnd
