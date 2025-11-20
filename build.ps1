$ErrorActionPreference = "Stop"

# ----- SETTINGS -----
$ToolchainFile = "mingw-toolchain.cmake"
$BuildDir = "build-windows"

# ----- WINDOWS BUILD -----
Write-Host "=== Building Windows version ===" -ForegroundColor Green

if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

Set-Location $BuildDir

# Clean only CMake files
Remove-Item -Force -ErrorAction SilentlyContinue `
    CMakeFiles, CMakeCache.txt, cmake_install.cmake, Makefile, chip8.exe

cmake -DCMAKE_TOOLCHAIN_FILE="..\$ToolchainFile" ..
cmake --build . --config Release

Set-Location ..

Write-Host ""
Write-Host "✓ Windows build complete: $BuildDir\chip8.exe" -ForegroundColor Green