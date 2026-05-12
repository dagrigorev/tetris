param(
    [ValidateSet('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')]
    [string]$Configuration = 'Release',

    [string]$Generator = 'auto',
    [ValidateSet('x64', 'Win32', 'ARM64')]
    [string]$Architecture = 'x64',
    [string]$BuildDir = '',
    [switch]$Clean,
    [switch]$NoFetchSdl2
)

$ErrorActionPreference = 'Stop'

function Test-IsWindowsHost {
    return (($env:OS -eq 'Windows_NT') -or [System.Runtime.InteropServices.RuntimeInformation]::IsOSPlatform([System.Runtime.InteropServices.OSPlatform]::Windows))
}

function Invoke-NativeCommand {
    param(
        [Parameter(Mandatory = $true)][string]$FilePath,
        [Parameter(ValueFromRemainingArguments = $true)][string[]]$Arguments
    )

    & $FilePath @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code $LASTEXITCODE: $FilePath $($Arguments -join ' ')"
    }
}

function Get-CMakeGenerators {
    try {
        $json = & cmake -E capabilities 2>$null
        if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($json)) {
            return @()
        }
        $capabilities = $json | ConvertFrom-Json
        return @($capabilities.generators | ForEach-Object { $_.name })
    } catch {
        return @()
    }
}

function Resolve-VisualStudioGenerator {
    $available = Get-CMakeGenerators
    foreach ($candidate in @('Visual Studio 18 2026', 'Visual Studio 17 2022', 'Visual Studio 16 2019')) {
        if ($available -contains $candidate) {
            return $candidate
        }
    }

    $fallback = $available | Where-Object { $_ -like 'Visual Studio *' } | Select-Object -First 1
    if ($null -ne $fallback) {
        return [string]$fallback
    }

    return ''
}

function Find-VsWhere {
    $paths = @(
        Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe',
        Join-Path $env:ProgramFiles 'Microsoft Visual Studio\Installer\vswhere.exe'
    )

    foreach ($path in $paths) {
        if ($path -and (Test-Path $path)) {
            return $path
        }
    }

    $cmd = Get-Command vswhere.exe -ErrorAction SilentlyContinue
    if ($null -ne $cmd) {
        return $cmd.Source
    }

    return ''
}

function Import-VisualStudioEnvironment {
    param([string]$Arch)

    if ($env:VCToolsInstallDir -and (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
        return
    }

    $vswhere = Find-VsWhere
    if ([string]::IsNullOrWhiteSpace($vswhere)) {
        return
    }

    $installPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($installPath)) {
        return
    }

    $vsDevCmd = Join-Path $installPath 'Common7\Tools\VsDevCmd.bat'
    if (!(Test-Path $vsDevCmd)) {
        return
    }

    Write-Host "Importing MSVC environment: $installPath"
    $cmdLine = '"{0}" -arch={1} -host_arch={1} >nul && set' -f $vsDevCmd, $Arch
    $environment = & cmd.exe /s /c $cmdLine
    if ($LASTEXITCODE -ne 0) {
        throw 'Failed to import Visual Studio developer environment.'
    }

    foreach ($line in $environment) {
        $index = $line.IndexOf('=')
        if ($index -le 0) {
            continue
        }
        $name = $line.Substring(0, $index)
        $value = $line.Substring($index + 1)
        [Environment]::SetEnvironmentVariable($name, $value, 'Process')
    }
}

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = Resolve-Path (Join-Path $ScriptDir '..')
$IsWindowsHost = Test-IsWindowsHost

if ($Generator -eq 'auto') {
    if ($IsWindowsHost) {
        $vsGenerator = Resolve-VisualStudioGenerator
        if (![string]::IsNullOrWhiteSpace($vsGenerator)) {
            $Generator = $vsGenerator
        } else {
            $Generator = 'Ninja'
        }
    } else {
        $Generator = 'Ninja'
    }
}

$generatorSafeName = ($Generator -replace '[^A-Za-z0-9]+', '-').Trim('-').ToLowerInvariant()
if ([string]::IsNullOrWhiteSpace($BuildDir)) {
    $BuildDir = Join-Path $RepoRoot ("build\$generatorSafeName\" + $Configuration.ToLowerInvariant())
}
$DistDir = Join-Path $RepoRoot 'dist'

Write-Host 'Layered Games build script'
Write-Host "Repository   : $RepoRoot"
Write-Host "Build dir    : $BuildDir"
Write-Host "Dist dir     : $DistDir"
Write-Host "Configuration: $Configuration"
Write-Host "Generator    : $Generator"
if ($IsWindowsHost) {
    Write-Host "Architecture : $Architecture"
}

if ($IsWindowsHost -and ($Generator -match 'Ninja|NMake|Makefiles')) {
    Import-VisualStudioEnvironment -Arch $Architecture
}

if ($Clean) {
    if (Test-Path $BuildDir) {
        Write-Host "Removing build dir: $BuildDir"
        Remove-Item $BuildDir -Recurse -Force
    }
    if (Test-Path $DistDir) {
        Write-Host "Removing dist dir: $DistDir"
        Remove-Item $DistDir -Recurse -Force
    }
}

New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
New-Item -ItemType Directory -Force -Path $DistDir | Out-Null

$fetchSdl2 = if ($NoFetchSdl2) { 'OFF' } else { 'ON' }
$configureArgs = @(
    '-S', $RepoRoot,
    '-B', $BuildDir,
    '-G', $Generator,
    "-DLAYERED_GAMES_FETCH_SDL2=$fetchSdl2",
    "-DLAYERED_GAMES_DIST_DIR=$DistDir"
)

if ($IsWindowsHost -and ($Generator -like 'Visual Studio *')) {
    $configureArgs += @('-A', $Architecture)
}

# Single-config generators such as Ninja and NMake need CMAKE_BUILD_TYPE.
if ($Generator -notmatch 'Visual Studio|Xcode|Multi-Config') {
    $configureArgs += "-DCMAKE_BUILD_TYPE=$Configuration"

    # On Windows, plain clang++.exe from PATH often fails outside a MSVC developer environment
    # because oldnames.lib/msvcrt*.lib cannot be found. Prefer the imported MSVC compiler unless
    # the caller explicitly selected CC/CXX in the environment.
    if ($IsWindowsHost -and [string]::IsNullOrWhiteSpace($env:CC) -and [string]::IsNullOrWhiteSpace($env:CXX)) {
        $cl = Get-Command cl.exe -ErrorAction SilentlyContinue
        if ($null -ne $cl) {
            $configureArgs += @('-DCMAKE_C_COMPILER=cl', '-DCMAKE_CXX_COMPILER=cl')
        }
    }
}

Write-Host 'Configuring...'
Invoke-NativeCommand cmake @configureArgs

Write-Host 'Building...'
Invoke-NativeCommand cmake --build $BuildDir --config $Configuration --target layered_games --parallel

# CMake post-build commands copy the executable and SDL2 runtime into dist.
# This fallback keeps the script useful even when a custom generator skips post-build copy.
$exeName = if ($IsWindowsHost) { 'layered_games.exe' } else { 'layered_games' }
$exe = Get-ChildItem -Path $BuildDir -Filter $exeName -Recurse -File -ErrorAction SilentlyContinue |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 1
if ($null -ne $exe) {
    Copy-Item $exe.FullName -Destination (Join-Path $DistDir $exeName) -Force
}

if ($IsWindowsHost) {
    Get-ChildItem -Path $BuildDir -Filter 'SDL2.dll' -Recurse -File -ErrorAction SilentlyContinue |
        Sort-Object FullName -Unique |
        ForEach-Object { Copy-Item $_.FullName -Destination (Join-Path $DistDir $_.Name) -Force }
}

Write-Host ''
Write-Host 'Build completed.'
Write-Host "Run from: $DistDir"
Write-Host "Executable: $(Join-Path $DistDir $exeName)"
