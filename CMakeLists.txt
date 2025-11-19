cmake_minimum_required(VERSION 3.16)
project(chip8)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(include)

# Platform detection
if(WIN32)
  # Windows (native or cross-compile)
  set(SDL2_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build-windows/SDL2-2.28.5/x86_64-w64-mingw32/include)
  set(SDL2_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/build-windows/SDL2-2.28.5/x86_64-w64-mingw32/lib/libSDL2.a)
  include_directories(${SDL2_INCLUDE_DIR})
  
  add_executable(chip8
    src/main.c
    src/chip8.c
    src/display.c
  )
  
  target_link_libraries(chip8 PRIVATE 
    ${SDL2_LIBRARY} 
    m
    setupapi
    winmm
    imm32
    version
    cfgmgr32
  )
else()
  # Linux/Unix
  find_package(SDL2 REQUIRED)
  
  add_executable(chip8
    src/main.c
    src/chip8.c
    src/display.c
  )
  
  target_link_libraries(chip8 PRIVATE SDL2::SDL2 SDL2::SDL2main m)
endif()