set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# SDL2 paths - adjust the path to match your setup
set(SDL2_INCLUDE_DIR /home/scythe/C-Projects/chip-8/build-windows/SDL2-2.28.5/x86_64-w64-mingw32/include)
set(SDL2_LIBRARY /home/scythe/C-Projects/chip-8/build-windows/SDL2-2.28.5/x86_64-w64-mingw32/lib/libSDL2.a)