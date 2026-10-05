; installer/odri.iss
;
; Inno Setup installer for Odri Windows TSF.

; ===========================================================================
; Application
; ===========================================================================

#ifndef AppVersion
#define AppVersion "0.0.0"
#endif

#define AppName             "Odri"
#define AppVersionString    AppVersion
#define AppPublisher        "Odrik"
#define AppCopyright        "Copyright (C) 2026 H.M. Tajul Islam Tanim"
#define AppPublisherURL     "https://github.com/odrik-bn"
#define AppSupportURL       "https://github.com/odrik-bn/odri-windows"
#define AppUpdatesURL       "https://github.com/odrik-bn/odri-windows"


; ===========================================================================
; Files
; ===========================================================================

#define AppDllName          "odri_tsf.dll"
#define AppIconName         "odri.ico"

#define BuildDir            "..\build\Release"
#define SourceDir           "..\src"
#define ResourcesDir        SourceDir + "\resources"
#define ScriptsDir          "..\scripts"

#define DllSource           BuildDir + "\" + AppDllName
#define IconSource          ResourcesDir + "\" + AppIconName

#define RegisterScript      ScriptsDir + "\register.ps1"
#define UninstallScript     ScriptsDir + "\uninstall.ps1"
#define CommonScript        ScriptsDir + "\common.ps1"

#define LicenseSource       "..\LICENSE"


; ===========================================================================
; Installer
; ===========================================================================

#define InstallDir          "{autopf}\Odri"
#define OutputDir           "Output"
#define OutputName          "OdriSetup-x64"


; ===========================================================================
; Setup
; ===========================================================================

[Setup]

AppId={{6B2B9C2E-6E3B-4B7B-9C9A-6C6E9E6B3E4A}

AppName={#AppName} by {#AppPublisher}
AppVersion={#AppVersionString}
AppVerName={#AppName} {#AppVersionString}

AppPublisher={#AppPublisher}
AppCopyright={#AppCopyright}

AppPublisherURL={#AppPublisherURL}
AppSupportURL={#AppSupportURL}
AppUpdatesURL={#AppUpdatesURL}

DefaultDirName={#InstallDir}
DefaultGroupName={#AppName}

DisableDirPage=yes
DisableProgramGroupPage=yes
DisableWelcomePage=no

ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

PrivilegesRequired=admin

SetupIconFile={#IconSource}
UninstallDisplayIcon={app}\{#AppIconName}

LicenseFile={#LicenseSource}

OutputDir={#OutputDir}
OutputBaseFilename={#OutputName}

Compression=lzma2
SolidCompression=yes
WizardStyle=modern
SetupLogging=yes


; ===========================================================================
; Language
; ===========================================================================

[Languages]

Name: "english"; MessagesFile: "compiler:Default.isl"


; ===========================================================================
; Messages
; ===========================================================================

[Messages]

WelcomeLabel1=Welcome to the Odri Setup Wizard
WelcomeLabel2=This will install Odri, a Bangla input engine for Windows.%n%nCopyright (C) 2026 H.M. Tajul Islam Tanim.%nOdri is licensed under the GNU General Public License v3.0.


; ===========================================================================
; Installed files
; ===========================================================================

[Files]

; Odri Windows TSF
Source: "{#DllSource}"; \
    DestDir: "{app}"; \
    Flags: ignoreversion

; Application icon
Source: "{#IconSource}"; \
    DestDir: "{app}"; \
    Flags: ignoreversion

; Registration scripts
Source: "{#RegisterScript}"; \
    DestDir: "{app}\scripts"; \
    Flags: ignoreversion

Source: "{#UninstallScript}"; \
    DestDir: "{app}\scripts"; \
    Flags: ignoreversion

Source: "{#CommonScript}"; \
    DestDir: "{app}\scripts"; \
    Flags: ignoreversion


; ===========================================================================
; Installation
; ===========================================================================

[Run]

Filename: "{sys}\WindowsPowerShell\v1.0\powershell.exe"; \
    Parameters: "-NoProfile -ExecutionPolicy Bypass -File ""{app}\scripts\register.ps1"" -Dll ""{app}\{#AppDllName}"""; \
    StatusMsg: "Registering {#AppName}..."; \
    Flags: runhidden waituntilterminated


; ===========================================================================
; Uninstallation
; ===========================================================================

[UninstallRun]

Filename: "{sys}\WindowsPowerShell\v1.0\powershell.exe"; \
    Parameters: "-NoProfile -ExecutionPolicy Bypass -File ""{app}\scripts\uninstall.ps1"" -Dll ""{app}\{#AppDllName}"""; \
    StatusMsg: "Unregistering {#AppName}..."; \
    Flags: runhidden waituntilterminated