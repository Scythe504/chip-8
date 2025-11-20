# CHIP-8 Interpreter

A CHIP-8 interpreter written in C with SDL2 graphics.

### Direct Installation

#### Linux(amd64)
```bash
wget https://github.com/Scythe504/chip-8/releases/download/v1.0.0/chip8
chmod +x chip8
```
#### Windows(amd64)
```ps1
curl -L -o chip8.exe https://github.com/Scythe504/chip-8/releases/download/v1.0.0/chip8.exe
```
### ROMS (Chip8)
[Roms](https://github.com/scythe504/chip-8/tree/main/roms)


Refer to [Usage](#usage) or [Controls](#controls)

### Prerequisites (for compilation)

#### For Linux
- GCC or Clang
- CMake 3.16+
- SDL2 development libraries

#### For Windows
- MinGW-w64
- CMake 3.16+
- wget or curl (for downloading SDL2)

### Installation

#### Linux

##### Using build script
```bash
chmod +x build.sh
./build.sh
```

Builds both Linux and Windows executables. Requires Windows SDL2 in `build-windows/SDL2-2.28.5/` when running on WSL2 (see [Windows installation](#windows)).
#### Manual build
```bash
sudo apt install libsdl2-dev cmake build-essential
mkdir build-linux && cd build-linux
cmake ..
make
```

#### Windows

##### Using build script
```powershell
.\build.ps1
```

##### Manual build with SDL2 setup
```cmd
cd build-windows
wget https://github.com/libsdl-org/SDL/releases/download/release-2.28.5/SDL2-devel-2.28.5-mingw.tar.gz
tar xzf SDL2-devel-2.28.5-mingw.tar.gz

cmake -DCMAKE_TOOLCHAIN_FILE=..\mingw-toolchain.cmake ..
cmake --build .
```

### Usage

Run with a CHIP-8 ROM file:
```bash
./chip8 path/to/rom.ch8
```


#### Controls
```
1 2 3 4       (hex keys 0x1-0x4, 0xC)
Q W E R       (hex keys 0x4-0x7, 0xD)
A S D F       (hex keys 0x7-0xA, 0xE)
Z X C V       (hex keys 0xA, 0x0, 0xB, 0xF)
```

### References

- [Tobias V. Langhoff - Guide to making a CHIP-8 emulator](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/)
- [CHIP-8 Wikipedia](https://en.wikipedia.org/wiki/CHIP-8)
- [SDL2 Documentation](https://wiki.libsdl.org/SDL2/FrontPage)