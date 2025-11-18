#include "display.h"

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "chip8.h"

#define BLACK 0
#define WHITE 255

SDL_Window* display_window = NULL;
SDL_Renderer* display_renderer = NULL;

void display_init() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    perror("SDL2 could not be initialize video subsystem\n");
    exit(1);
  }

  display_window = SDL_CreateWindow("Chip8 Emu", 0, 0, SCREEN_W, SCREEN_H, SDL_WINDOW_OPENGL);

  if (display_window == NULL) {
    perror("SDL_Window could not be created\n");
    exit(1);
  }

  display_renderer = SDL_CreateRenderer(display_window,
                                        -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (display_renderer == NULL) {
    perror("SDL_Renderer could not be created\n");
    exit(1);
  }
}

void display_cleanup() {
  SDL_DestroyWindow(display_window);

  SDL_Quit();
}

bool display_poll_events() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT)
      return true;

    // TODO: keyboard handling here (set key[] in chip8)
  }

  return false;
}

void display_render(const uint8_t screen[SCREEN_H][SCREEN_W]) {

}