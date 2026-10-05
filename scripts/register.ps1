# scripts/register.ps1
#
# Registers the Odri TSF DLL with Windows. Intended to run as an Inno
# Setup post-install [Run] step, right after Setup has copied the files:
#
#   [Run]
#   Filename: "powershell.exe"; \
#     Parameters: "-NoProfile -ExecutionPolicy Bypass -File ""{app}\scripts\register.ps1"" -Dll ""{app}\odri_tsf.dll"""; \
#     Flags: runhidden waituntilterminated
#
# Also usable directly, e.g. by dev-build.ps1:
#
#   powershell -ExecutionPolicy Bypass -File scripts\register.ps1 -Dll "build\Debug\odri_tsf.dll"
#
# common.ps1 must ship next to this file (Inno needs to package it into
# {app}\scripts, not just register.ps1/uninstall.ps1) since both scripts
# dot-source it.

param(
    [string]$Dll
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')

Assert-Administrator

if ([string]::IsNullOrWhiteSpace($Dll)) {
    $dllPath = Join-Path $OdriInstallDir $OdriDllName
}
else {
    $dllPath = (Resolve-Path -LiteralPath $Dll -ErrorAction Stop).Path
}

Write-Step 'Stopping Windows components...'
Stop-OdriProcesses

Write-Step 'Removing previous Odri registration...'
# Unregister first so upgrades (same path, new file) re-register cleanly.
# Ignored if nothing was registered yet - this may be a first install.
Unregister-OdriDll -DllPath $dllPath
Write-Step 'Registering Odri...'
$exitCode = Register-OdriDll -DllPath $dllPath

exit $exitCode
