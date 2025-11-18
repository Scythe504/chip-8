cmake_minimum_required(VERSION 3.16)
project(chip8)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(include)

find_package(SDL2 REQUIRED)

add_executable(chip8
  src/main.c
  src/chip8.c
  src/display.c
)

target_link_libraries(chip8 PRIVATE SDL2::SDL2 SDL2::SDL2main)