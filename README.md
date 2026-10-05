# Odri for Windows

**Odri Phonetic** is a Bangla phonetic input method for Windows.

Type Bangla phonetically using Latin letters and Odri converts your input into Bangla script in real time.

For example:

```text
ami banglay likhi
```

becomes:

```text
আমি বাংলায় লিখি
```

Odri integrates with Windows through the **Text Services Framework (TSF)**, so it works as a native Windows keyboard/input method.

---

# Features

* Bangla phonetic typing
* Real-time Latin-to-Bangla transliteration
* Native Windows TSF integration
* Works across Windows applications that support TSF
* Backspace-aware composition editing
* Powered by [odri-core](https://github.com/odri-bn/odri-core)
* No Python, Node.js, Java, or other runtime required
* Prebuilt releases for end users
* Native x64 Windows installer
* Portable ZIP release

---

# Installation

## Requirements

* Windows 10 or later
* 64-bit Windows
* Administrator privileges

There are two recommended ways to install Odri.

---

## Method 1 : Windows Installer

The easiest method is to download the latest **`OdriSetup-x64.exe`** from the project's GitHub Releases page.

The installer will:

1. Install Odri into the system.
2. Install the Odri TSF DLL.
3. Register the TSF component with Windows.
4. Create the required installation files.
5. Configure the application for normal Windows use.

The installer is the recommended method for normal users.

---

## Method 2 : Online PowerShell Installer

Odri also provides a PowerShell installer that downloads the latest stable release automatically.

Open **PowerShell** and run:

```powershell
irm https://raw.githubusercontent.com/odri-bn/odri-windows/main/scripts/install.ps1 | iex
```

The installer will automatically:

1. Detect your Windows architecture.
2. Download the latest stable Odri release.
3. Install the Odri TSF component.
4. Register Odri with Windows.
5. Finish the installation.

You do **not** need to download or provide a DLL manually.

> **Note:** This command downloads and executes a PowerShell script from the Odri GitHub repository. Only use it if you trust the repository.

---

# Enable Odri Phonetic

After installation, select **Odri Phonetic** from the Windows keyboard/language selector.
The default shortcut for windows keyboard toggle is <kbd>WIN</kbd> + <kbd>&lt;space&gt;</kbd>.

You can then start typing Bangla phonetically.

---

# Using Odri

Once Odri Phonetic is selected, type using Latin letters.

For example:

```text
ami
```

produces:

```text
আমি
```

You can continue typing naturally:

```text
ami banglay likhte pari
```

---

# How It Works

Odri uses a rule-based transliteration engine.

The general processing pipeline is:

```text
                    Odri Windows
                          │
                          ▼
                    Windows TSF
                          │
                          ▼
                    odri-core
                          │
                          ▼
                  Bangla composition
```

The Windows component receives keyboard input and maintains the current Latin composition.

The transliteration work itself is performed by [odri-core](https://github.com/odri-bn/odri-core).

---

# Releases

Prebuilt Windows binaries are distributed through GitHub Releases.

A normal release contains:

```text
OdriSetup-x64.exe
odri-windows-x64.zip
```

### OdriSetup-x64.exe

The standard Windows installer.

Use this if you want a normal graphical installation.

### odri-windows-x64.zip

The portable release package.

It contains the compiled TSF component:

```text
odri-windows-x64.zip
└── odri_tsf.dll
```

The ZIP is primarily useful for developers, advanced users, and manual installation.

---

# Manual Installation

Manual installation is primarily useful for developers or users who already have a compiled Odri DLL.

If you have:

```text
odri_tsf.dll
```

you can install it with:

```powershell
.\scripts\install.ps1 -Dll path\to\odri_tsf.dll
```

For example:

```powershell
.\scripts\install.ps1 -Dll build\Debug\odri_tsf.dll
```

The script must be run with administrator privileges.

---

# Uninstallation

If Odri was installed normally, use the Windows installed-apps interface or the provided uninstaller.

For a script-based uninstall:

```powershell
.\scripts\uninstall.ps1
```

The uninstaller removes the Odri TSF registration and installed components.

After uninstalling, Odri Phonetic will no longer appear as an available Windows keyboard.

---

# Development Setup

## Requirements

To build Odri Windows from source, install:

* Windows 10 or later
* 64-bit Windows
* Git
* CMake 3.21 or later
* Visual Studio with C++ desktop development tools
* Windows SDK
* PowerShell

Inno Setup 6 is additionally required if you want to build the Windows installer.

---

## Clone the Repository

Clone the repository with its submodules:

```powershell
git clone --recurse-submodules https://github.com/odri-bn/odri-windows.git
cd odri-windows
```

If you already cloned the repository without submodules:

```powershell
git submodule update --init --recursive
```

The `odri-core` project is included as a Git submodule.

---

# Repository Structure

```text
odri-windows/
│
├── CMakeLists.txt
│
├── installer/
│   └── odri.iss
│
├── external/
│   └── odri-core/
│
├── src/
│   ├── app/
│   │   └── ...
│   │
│   ├── core/
│   │   ├── engine_host.cpp
│   │   └── engine_host.hpp
│   │
│   ├── tsf/
│   │   ├── text_service.cpp
│   │   ├── text_service.hpp
│   │   └── ...
│   │
│   ├── util/
│   │   └── ...
│   │
│   └── windows/
│       └── ...
│
├── tests/
│   └── engine_host_tests.cpp
│
├── scripts/
│   ├── common.ps1
│   ├── dev-build.ps1
│   ├── dev-clean.ps1
│   ├── install.ps1
│   ├── register.ps1
│   ├── release.ps1
│   └── uninstall.ps1
│
└── .github/
    └── workflows/
        └── release.yml
```

---

# Build

Configure the project:

```powershell
cmake -S . -B build -A x64
```

Build the Debug configuration:

```powershell
cmake --build build --config Debug
```

The resulting TSF DLL will be located at:

```text
build\Debug\odri_tsf.dll
```

---

# Release Build

The release configuration uses MSVC Release optimizations including:

```text
/O2
/GL
/Gy
/Gw
/LTCG
/OPT:REF
/OPT:ICF
```

The release build also disables logging and tests by default and uses the static MSVC runtime.

To build the release configuration manually:

```powershell
cmake -S . -B build -A x64 `
    -DODRI_WINDOWS_ENABLE_LOGGING=OFF `
    -DODRI_WINDOWS_BUILD_TESTS=OFF `
    -DODRI_WINDOWS_STATIC_RUNTIME=ON

cmake --build build --config Release --target odri_tsf
```

The resulting DLL is:

```text
build\Release\odri_tsf.dll
```

The release build does not use CPU-specific options such as `/arch:AVX2` or `/arch:AVX512`, allowing the binary to target general x64 Windows systems.

---

# Run Tests

Enable tests when configuring the project:

```powershell
cmake -S . -B build -A x64 `
    -DODRI_WINDOWS_BUILD_TESTS=ON
```

Build:

```powershell
cmake --build build --config Debug
```

Run:

```powershell
ctest --test-dir build -C Debug --output-on-failure
```

The tests cover the platform-independent `EngineHost` interface and its interaction with `odri-core`.

---

# Developer Installation

After building the project, the local DLL can be registered with:

```powershell
.\scripts\install.ps1 -Dll build\Debug\odri_tsf.dll
```

This is useful when developing the TSF implementation because you can build and immediately register the newly compiled DLL.

The developer build helper can also be used to automate the build and replacement process.

For normal end-user installation, use the Windows installer or online installer instead.

---

# Building a Release

The release process is automated by:

```text
scripts\release.ps1
```

Run it locally with:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\release.ps1
```

A version can be supplied explicitly:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\release.ps1 -Version 1.2.0
```

The release script:

1. Builds the Release configuration.
2. Creates the portable ZIP package.
3. Compiles the Inno Setup installer.
4. Places all release artifacts in `dist\`.

The resulting directory contains:

```text
dist/
├── OdriSetup-x64.exe
└── odri-windows-x64.zip
```

These files can then be uploaded to a GitHub Release.

---

# Automated GitHub Releases

Odri uses GitHub Actions to build releases automatically.

The workflow is located at:

```text
.github/workflows/release.yml
```

A release is triggered by pushing a version tag.

For example:

```powershell
git add .
git commit -m "Release 1.2.0"
git push origin main

git tag v1.2.0
git push origin v1.2.0
```

The `v1.2.0` tag triggers the release workflow.

GitHub Actions then:

```text
Git tag
   │
   ▼
GitHub Actions
   │
   ├── Checkout repository
   ├── Checkout odri-core submodule
   ├── Configure CMake
   ├── Build optimized Release DLL
   ├── Build Inno Setup installer
   ├── Create portable ZIP
   └── Create GitHub Release
```

The resulting GitHub Release contains:

```text
OdriSetup-x64.exe
odri-windows-x64.zip
```

No manual build is required for the official release.

---

# Architecture

Odri Windows consists of several layers:

```text
┌─────────────────────────────────────┐
│             Windows Apps            │
└──────────────────┬──────────────────┘
                   │
                   ▼
┌─────────────────────────────────────┐
│       Windows Text Services         │
│              Framework              │
└──────────────────┬──────────────────┘
                   │
                   ▼
┌─────────────────────────────────────┐
│         Odri Text Service          │
│                                     │
│  Keyboard events / composition /    │
│  TSF communication                  │
└──────────────────┬──────────────────┘
                   │
                   ▼
┌─────────────────────────────────────┐
│            EngineHost               │
│                                     │
│  Windows ↔ Odri Core boundary     │
└──────────────────┬──────────────────┘
                   │
                   ▼
┌─────────────────────────────────────┐
│            odri-core              │
│                                     │
│  Tokenization / rules / parsing /   │
│  transliteration / rendering        │
└─────────────────────────────────────┘
```

The Windows project is responsible for Windows integration.

The core project is responsible for transliteration.

---

# Troubleshooting

## Odri Does Not Appear in the Keyboard List

Make sure that:

1. Installation completed successfully.
2. Odri was registered with administrator privileges.
3. You are using 64-bit Windows.
4. You are looking under the Bangla keyboard settings.

Try restarting Windows if the keyboard does not immediately appear.

---

## The Installer Says That the DLL Is Locked

Close applications that may currently be using the Odri TSF.

The installer normally stops relevant processes before registering or replacing the DLL.

If necessary, restart Windows and run the installer again.

---

## Odri Appears but Does Not Produce Bangla

Make sure **Odri Phonetic** is selected as the active keyboard.

Also verify that you are typing Latin input into an application that supports Windows text services.

---

## The PowerShell Installer Does Not Run

If PowerShell reports an execution-policy restriction, you can run the installer explicitly with:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\install.ps1
```

For the online installer, use the command documented in the **Online PowerShell Installer** section.

---

# Security

The online installation command downloads and executes a PowerShell script:

```powershell
irm https://raw.githubusercontent.com/odri-bn/odri-windows/main/scripts/install.ps1 | iex
```

Only run this command if you trust the Odri repository.

Users who prefer not to execute a remote PowerShell script can download `OdriSetup-x64.exe` from GitHub Releases and install Odri using the graphical installer.

---

# Related Projects

* **[odri-core](https://github.com/odri-bn/odri-core)** : Odri's platform-independent transliteration engine.
* **[odri-cli](https://github.com/odri-bn/odri-cli)** : Command-line interface for Odri.
* **[odri-android](https://github.com/odri-bn/odri-android)** : Android keyboard integration.

---

# Contributing

Contributions, bug reports, feature requests, and improvements are welcome.

Before contributing, please make sure the project builds successfully and that existing tests pass.

```powershell
cmake --build build --config Debug

ctest --test-dir build -C Debug --output-on-failure
```

For development and implementation details, see the project's contribution documentation.

---

# License

See the repository license for licensing information.
