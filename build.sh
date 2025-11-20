#!/bin/bash
set -e

# ----- SETTINGS -----
WIN_SDL_DIR="build-windows/SDL2-2.28.5/x86_64-w64-mingw32"
TOOLCHAIN_FILE="mingw-toolchain.cmake"

# ----- LINUX BUILD -----
echo "=== Building Linux version ==="
mkdir -p build-linux
cd build-linux
rm -rf CMakeFiles CMakeCache.txt cmake_install.cmake Makefile chip8
cmake ..
make -j$(nproc)
cd ..

echo "✔ Linux build complete: build-linux/chip8"

# ----- WINDOWS BUILD -----
echo "=== Building Windows version (MinGW) ==="
mkdir -p build-windows
cd build-windows

# Only clean CMake files, not SDL2 directory
rm -rf CMakeFiles CMakeCache.txt cmake_install.cmake Makefile chip8.exe

cmake -DCMAKE_TOOLCHAIN_FILE=../${TOOLCHAIN_FILE} ..
make -j$(nproc)

cd ..

echo "✔ Windows build complete: build-windows/chip8.exe"
echo ""
echo "Ready to distribute:"
echo "  - build-linux/chip8"
echo "  - build-windows/chip8.exe"