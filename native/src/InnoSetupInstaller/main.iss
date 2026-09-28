#include "paths.iss"
#include "app-version.iss"
#include "app-defines.iss"
#include "helpers/IncludeSrcDirIfExists.iss"
#ifndef MyAppInstallerFilename
   #define MyAppInstallerFilename MyAppName + "Setup"
   #ifndef OPTION_EXCLUDE_PDB
      #define MyAppInstallerFilename MyAppInstallerFilename + "WithPDB"
   #endif
#endif

#emit Message("Compiling installer: {#MyAppInstallerFilename}.exe...")

[Setup]
#if VER >= 0x05010000
   #if VER >= 0x06030000
      ArchitecturesAllowed = x64compatible
      ArchitecturesInstallIn64BitMode = x64compatible
   #else
      ArchitecturesAllowed = x64
      ArchitecturesInstallIn64BitMode = x64
   #endif
#endif
#if VER >= 0x07000000
   SetupArchitecture = x64
#endif
AppId        = {#MyAppId}
AppName      = {#MyAppName}
AppVersion   = "{#app_version_major}.{#app_version_minor}.{#app_version_patch}.{#app_version_build}"
AppPublisher = {#MyAppPublisher}
#ifdef MyAppURL
   AppPublisherURL = {#MyAppURL}
   AppSupportURL   = {#MyAppURL}
   AppUpdatesURL   = {#MyAppURL}
#endif
CreateAppDir       = yes
DefaultDirName     = {autopf}\{#MyAppName}
PrivilegesRequiredOverridesAllowed = dialog
OutputDir          = {#InstallerBuildResultPath}
OutputBaseFilename = {#MyAppInstallerFilename}
SetupIconFile      = {#CppProjectPath}\{#MyAppName}.ico
Compression        = lzma
SolidCompression   = yes
WizardStyle        = modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
#define CppOutputExclusions "\Qt5*.dll"
#ifdef OPTION_EXCLUDE_PDB
   #define CppOutputExclusions CppOutputExclusions + ",*.pdb"
#endif
Source: "{#CppBuildResultPath}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs; Excludes: "{#CppOutputExclusions}"
{#IncludeSrcDirIfExists("LICENSES")}
{#IncludeSrcDirIfExists("help", 1)}
{#IncludeSrcDirIfExists("userdata", 1, "\scripts\developer tests,\script-packages\-developer-tests-")}

[UninstallDelete]
//Type: files; Name: "{app}\ReachVariantTool.ini"

; Install VC redist
#include "vc-install.iss"

// Uncomment to debug preprocessor output.
#expr SaveToFile(AddBackslash(SourcePath) + "__preprocessed.iss")
