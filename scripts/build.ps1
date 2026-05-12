param(
    [ValidateSet('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')]
    [string]$Configuration = 'Release',

    [string]$Generator = 'Ninja',
    [string]$BuildDir = '',
    [switch]$Clean,
    [switch]$NoFetchSdl2
)

$ErrorActionPreference = 'Stop'

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = Resolve-Path (Join-Path $ScriptDir '..')
if ([string]::IsNullOrWhiteSpace($BuildDir)) {
    $BuildDir = Join-Path $RepoRoot ("build\" + $Configuration.ToLowerInvariant())
}
$DistDir = Join-Path $RepoRoot 'dist'

Write-Host 'Layered Games build script'
Write-Host "Repository   : $RepoRoot"
Write-Host "Build dir    : $BuildDir"
Write-Host "Dist dir     : $DistDir"
Write-Host "Configuration: $Configuration"
Write-Host "Generator    : $Generator"

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

# Single-config generators such as Ninja need CMAKE_BUILD_TYPE.
if ($Generator -notmatch 'Visual Studio|Xcode|Multi-Config') {
    $configureArgs += "-DCMAKE_BUILD_TYPE=$Configuration"
}

Write-Host 'Configuring...'
& cmake @configureArgs

Write-Host 'Building...'
& cmake --build $BuildDir --config $Configuration --target layered_games --parallel

# CMake post-build commands copy the executable and SDL2 runtime into dist.
# This fallback keeps the script useful even when a custom generator skips post-build copy.
$exeName = if ($IsWindows -or $env:OS -eq 'Windows_NT') { 'layered_games.exe' } else { 'layered_games' }
$exe = Get-ChildItem -Path $BuildDir -Filter $exeName -Recurse -File -ErrorAction SilentlyContinue |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 1
if ($null -ne $exe) {
    Copy-Item $exe.FullName -Destination (Join-Path $DistDir $exeName) -Force
}

if ($IsWindows -or $env:OS -eq 'Windows_NT') {
    Get-ChildItem -Path $BuildDir -Filter 'SDL2.dll' -Recurse -File -ErrorAction SilentlyContinue |
        Sort-Object FullName -Unique |
        ForEach-Object { Copy-Item $_.FullName -Destination (Join-Path $DistDir $_.Name) -Force }
}

Write-Host ''
Write-Host 'Build completed.'
Write-Host "Run from: $DistDir"
Write-Host "Executable: $(Join-Path $DistDir $exeName)"
