cmake_minimum_required(VERSION 3.16)
project(chip8)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(include)

# Platform detection
if (MINGW OR WIN32)
  message(STATUS "Configuring Windows / MinGW build")

  # Try to find SDL2 in system paths first
  find_package(SDL2 QUIET)
  
  if (NOT SDL2_FOUND)
    # Fall back to local SDL2
    message(STATUS "SDL2 not found in system, using local SDL2")
    set(SDL2_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build-windows/SDL2-2.28.5/x86_64-w64-mingw32/include)
    set(SDL2_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/build-windows/SDL2-2.28.5/x86_64-w64-mingw32/lib/libSDL2.a)
    set(SDL2MAIN_LIB ${CMAKE_CURRENT_SOURCE_DIR}/build-windows/SDL2-2.28.5/x86_64-w64-mingw32/lib/libSDL2main.a)
    include_directories(${SDL2_INCLUDE_DIR})
  endif()

  add_executable(chip8
    src/main.c
    src/chip8.c
    src/display.c
    src/audio.c
  )

  if (SDL2_FOUND)
    target_link_libraries(chip8 PRIVATE SDL2::SDL2 SDL2::SDL2main m)
  else()
    target_link_libraries(chip8 PRIVATE 
      ${SDL2MAIN_LIB}
      ${SDL2_LIBRARY} 
      m
      setupapi
      winmm
      imm32
      version
      cfgmgr32
    )
  endif()
else()
  # Linux/Unix
  find_package(SDL2 REQUIRED)
  
  add_executable(chip8
    src/main.c
    src/chip8.c
    src/display.c
    src/audio.c
  )
  
  target_link_libraries(chip8 PRIVATE SDL2::SDL2 SDL2::SDL2main m)
endif()