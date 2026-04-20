param(
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
)

$ErrorActionPreference = "Stop"

function Write-Step {
    param([string]$Message)
    Write-Host "==> $Message" -ForegroundColor Cyan
}

function Invoke-Checked {
    param(
        [string]$FilePath,
        [string[]]$ArgumentList,
        [string]$WorkingDirectory = $RepoRoot
    )

    Write-Host "$FilePath $($ArgumentList -join ' ')"
    & $FilePath @ArgumentList
    if ($LASTEXITCODE -ne 0) {
        throw ("Command failed with exit code {0}: {1} {2}" -f $LASTEXITCODE, $FilePath, ($ArgumentList -join ' '))
    }
}

function Get-MSBuildPath {
    $vsWhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vsWhere) {
        $installationPath = & $vsWhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
        if ($installationPath) {
            $msbuild = Join-Path $installationPath "MSBuild\Current\Bin\MSBuild.exe"
            if (Test-Path $msbuild) {
                return $msbuild
            }
        }
    }

    $fallback = "MSBuild.exe"
    return $fallback
}

function Get-CMakePath {
    $vsWhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vsWhere) {
        $installationPath = & $vsWhere -latest -products * -property installationPath
        if ($installationPath) {
            $cmake = Join-Path $installationPath "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
            if (Test-Path $cmake) {
                return $cmake
            }
        }
    }

    $fallback = "cmake.exe"
    return $fallback
}

function Ensure-Dir {
    param([string]$Path)
    if (-not (Test-Path $Path)) {
        New-Item -ItemType Directory -Path $Path | Out-Null
    }
}

$RepoRoot = (Resolve-Path $RepoRoot).Path
$VendorBin = Join-Path $RepoRoot "vendor\bin"
$PremakeExe = Join-Path $VendorBin "premake5.exe"
$PremakeZip = Join-Path $VendorBin "premake5-windows.zip"
$PremakeUrl = "https://github.com/premake/premake-core/releases/download/v5.0.0-beta2/premake-5.0.0-beta2-windows.zip"
$AssimpSource = Join-Path $RepoRoot "QuentlamEngine\vendor\assimp"
$AssimpBuild = Join-Path $AssimpSource "build"
$AssimpSolution = Join-Path $AssimpBuild "Assimp.sln"
$MSBuildExe = Get-MSBuildPath
$CMakeExe = Get-CMakePath

Ensure-Dir $VendorBin

if (-not (Test-Path $PremakeExe)) {
    Write-Step "Downloading premake5"
    Invoke-WebRequest -Uri $PremakeUrl -OutFile $PremakeZip
    Expand-Archive -LiteralPath $PremakeZip -DestinationPath $VendorBin -Force
}

Write-Step "Configuring assimp with CMake"
Ensure-Dir $AssimpBuild
Push-Location $AssimpBuild
try {
    Invoke-Checked -FilePath $CMakeExe -ArgumentList @(
        "..",
        "-A", "x64",
        "-DBUILD_SHARED_LIBS=OFF",
        "-DASSIMP_BUILD_TESTS=OFF",
        "-DASSIMP_INSTALL=OFF",
        "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
}
finally {
    Pop-Location
}

Write-Step "Building assimp"
Push-Location $AssimpBuild
try {
    Invoke-Checked -FilePath $MSBuildExe -ArgumentList @(
        "Assimp.sln",
        "/m",
        "/p:Configuration=Debug;Platform=x64"
    )
    Invoke-Checked -FilePath $MSBuildExe -ArgumentList @(
        "Assimp.sln",
        "/m",
        "/p:Configuration=Release;Platform=x64"
    )
}
finally {
    Pop-Location
}

Write-Step "Generating Visual Studio 2022 solution"
Push-Location $RepoRoot
try {
    Invoke-Checked -FilePath $PremakeExe -ArgumentList @("vs2022")
}
finally {
    Pop-Location
}

Write-Step "Bootstrap complete"
