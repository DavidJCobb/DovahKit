
# Installer

This installer is built using Inno Setup 6.

The MSBuild project is built to reference a custom `.props` file, which defines:

* Custom C# code to pull the VC redistributable file version and overwrite `vc-version.iss`. The `windeployqt` tool copies the VC redistributable out of the current Visual Studio installation and into DovahKit's output directory (where the Inno Setup script then grabs it), so this custom C# build target is just querying that redistributable (the one in the VS installation) to feed its version number into the Inno Setup script.

* A property `$(InnoSetupCompilerPath)` defining the path to the Inno Setup 6 compiler (`ISCC.exe`).

In turn, the project file itself defines custom build steps for `main.iss` and `main-no-pdb.iss`, to compile the two installers.

Build output will be at `$(SolutionDir)\_output\InnoSetupInstaller\DovahKitSetup.exe`. The installer is always for the x64 Release version of DovahKit (I wrote the path directly into the installer script).
