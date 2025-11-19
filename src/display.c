#include "display.h"

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "chip8.h"

#define BLACK 0x000000
#define WHITE 0xFFFFFF
#define PIXEL_SIZE 10

SDL_Window* display_window = NULL;
SDL_Renderer* display_renderer = NULL;

void display_init() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    exit(1);
  }

  int win_w = SCREEN_W * PIXEL_SIZE;  // Extra space for debug panel
  int win_h = SCREEN_H * PIXEL_SIZE;

  display_window = SDL_CreateWindow("Chip8 Emu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                    win_w, win_h, 0);

  if (display_window == NULL) {
    exit(1);
  }

  display_renderer = SDL_CreateRenderer(display_window, -1, SDL_RENDERER_SOFTWARE);
  if (display_renderer == NULL) {
    exit(1);
  }

  printf("Renderer: software (no GPU involved)\n");

  SDL_RenderClear(display_renderer);
  SDL_RenderPresent(display_renderer);
}

void display_cleanup() {
  SDL_DestroyWindow(display_window);
  SDL_DestroyRenderer(display_renderer);
  SDL_Quit();
}

static uint8_t map_sdl_key(SDL_Keycode sym) {
  switch (sym) {
    case SDLK_1: return 0x1;
    case SDLK_2: return 0x2;
    case SDLK_3: return 0x3;
    case SDLK_4: return 0xC;
    case SDLK_q: return 0x4;
    case SDLK_w: return 0x5;
    case SDLK_e: return 0x6;
    case SDLK_r: return 0xD;
    case SDLK_a: return 0x7;
    case SDLK_s: return 0x8;
    case SDLK_d: return 0x9;
    case SDLK_f: return 0xE;
    case SDLK_z: return 0xA;
    case SDLK_x: return 0x0;
    case SDLK_c: return 0xB;
    case SDLK_v: return 0xF;
  }
  return 0xFF;
}

bool display_poll_events() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      return true;
    }

    if (e.type == SDL_KEYDOWN) {
      uint8_t key = map_sdl_key(e.key.keysym.sym);
      if (key != 0xFF)
        chip8_key_down(key);
    }
    if (e.type == SDL_KEYUP) {
      uint8_t key = map_sdl_key(e.key.keysym.sym);
      if (key != 0xFF) {
        chip8_key_up(key);
      }
    }
  }

  return false;
}

void display_render(const uint8_t* screen) {
  // Clear to black
  SDL_SetRenderDrawColor(display_renderer, 0, 0, 0, 255);
  SDL_RenderClear(display_renderer);

  // Draw CHIP-8 screen (64x32)
  SDL_SetRenderDrawColor(display_renderer, 255, 255, 255, 255);
  for (unsigned y = 0; y < SCREEN_H; y++) {
    for (unsigned x = 0; x < SCREEN_W; x++) {
      if (screen[SCREEN_IDX(y, x)]) {
        SDL_Rect rect = {
          x * PIXEL_SIZE,
          y * PIXEL_SIZE,
          PIXEL_SIZE,
          PIXEL_SIZE
        };
        SDL_RenderFillRect(display_renderer, &rect);
      }
    }
  }

  // Draw debug panel on the right
  int panel_x = SCREEN_W * PIXEL_SIZE + 10;
  int panel_y = 10;

  SDL_SetRenderDrawColor(display_renderer, 100, 100, 100, 255);
  SDL_Rect panel = {panel_x - 10, 0, 200, SCREEN_H * PIXEL_SIZE};
  SDL_RenderFillRect(display_renderer, &panel);

  // Simple text info (you can expand this)
  // For now just draw debug lines
  SDL_SetRenderDrawColor(display_renderer, 200, 200, 200, 255);
  SDL_RenderDrawLine(display_renderer, panel_x, panel_y, panel_x + 100, panel_y);
  SDL_RenderDrawLine(display_renderer, panel_x, panel_y + 20, panel_x + 100, panel_y + 20);
  SDL_RenderDrawLine(display_renderer, panel_x, panel_y + 40, panel_x + 100, panel_y + 40);

  SDL_RenderPresent(display_renderer);
}