param (
    [ValidateSet("x86", "x64")]
    [string]$Arch = "x64"
)

$ErrorActionPreference = "Stop"

# Ensure we're running inside Developer PowerShell
if (-not $env:VSCMD_VER) {
    Write-Host ""
    Write-Host "ERROR: This script must be run from a Visual Studio Developer PowerShell."
    Write-Host ""
    Write-Host "Please open:"
    Write-Host "  Start Menu -> Developer PowerShell for VS 2022"
    Write-Host ""
    exit 1
}

# Resolve paths relative to script location
$ScriptDir   = $PSScriptRoot
$ProjectRoot = Resolve-Path "$ScriptDir\.."
$DefFile     = Join-Path $ProjectRoot "src\iumbase.def"
$LibDir      = Join-Path $ProjectRoot "lib"
$OutputLib   = Join-Path $LibDir "iumbase.lib"

if (!(Test-Path $DefFile)) {
    Write-Error "DEF file not found at $DefFile"
    exit 1
}

# Create lib directory if needed
if (!(Test-Path $LibDir)) {
    New-Item -ItemType Directory -Path $LibDir | Out-Null
    Write-Host "Created directory: $LibDir"
}

# Map architecture
switch ($Arch) {
    "x86" { $Machine = "X86" }
    "x64" { $Machine = "X64" }
}

Write-Host "Generating import library..."
Write-Host "Architecture: $Machine"

& lib.exe /def:"$DefFile" /machine:$Machine /out:"$OutputLib"

if ($LASTEXITCODE -ne 0) {
    Write-Error "lib.exe failed with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}

Write-Host ""
Write-Host "Successfully generated:"
Write-Host "$OutputLib"
