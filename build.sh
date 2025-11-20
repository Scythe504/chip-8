#!/bin/bash
set -e

# ----- SETTINGS -----
WIN_SDL_DIR="build-windows/SDL2-2.28.5/x86_64-w64-mingw32"
TOOLCHAIN_FILE="mingw-toolchain.cmake"

# ----- LINUX BUILD -----
echo "=== Building Linux version ==="
mkdir -p build-linux
cd build-linux
rm -rf ./*
cmake ..
make -j$(nproc)
cd ..

echo "✔ Linux build complete: build-linux/chip8"

# ----- WINDOWS BUILD -----
echo "=== Building Windows version (MinGW) ==="
mkdir -p build-windows-out
cd build-windows-out
rm -rf ./*
cmake -DCMAKE_TOOLCHAIN_FILE=../${TOOLCHAIN_FILE} ..
make -j$(nproc)

# Copy required DLLs
if [ -f "../${WIN_SDL_DIR}/bin/SDL2.dll" ]; then
    cp "../${WIN_SDL_DIR}/bin/SDL2.dll" .
fi

cd ..

echo "✔ Windows build complete: build-windows-out/chip8.exe + SDL2.dll"
