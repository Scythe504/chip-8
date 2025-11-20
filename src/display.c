
#include "display.h"

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "chip8.h"

#define BLACK 0xFF000000
#define WHITE 0xFFFFFFFF

SDL_Window* display_window = NULL;
SDL_Renderer* display_renderer = NULL;
SDL_Texture* display_texture = NULL;
static uint32_t pixels[SCREEN_SIZE];

void display_init() {
  // initialize sdl
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    // fprintf(stderr, "SDL2 could not be initialize video subsystem: %s\n", SDL_GetError());
    exit(1);
  }

  // create window
  display_window = SDL_CreateWindow("Chip8 Emu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                    SCREEN_W * 10, SCREEN_H * 10, SDL_WINDOW_RESIZABLE);

  if (display_window == NULL) {
    // fprintf(stderr, "SDL_Window could not be created%s\n", SDL_GetError());
    exit(1);
  }

  // create renderer
  display_renderer = SDL_CreateRenderer(display_window, -1, 0);
  if (display_renderer == NULL) {
    // fprintf(stderr, "SDL_Renderer could not be created%s\n", SDL_GetError());
    exit(1);
  }

  SDL_RendererInfo info;
  SDL_GetRendererInfo(display_renderer, &info);
  printf("Renderer used: %s\n", info.name);

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
  SDL_RenderSetLogicalSize(display_renderer, SCREEN_W, SCREEN_H);
  // create texture
  display_texture = SDL_CreateTexture(display_renderer,
                                      SDL_PIXELFORMAT_ARGB8888,
                                      SDL_TEXTUREACCESS_STREAMING,
                                      SCREEN_W, SCREEN_H);

  if (display_texture == NULL) {
    // fprintf(stderr, "SDL_Texture could not be created%s\n", SDL_GetError());
    exit(1);
  }

  SDL_UpdateTexture(display_texture, NULL, pixels, SCREEN_W * sizeof(uint32_t));
  SDL_RenderClear(display_renderer);
  SDL_RenderCopy(display_renderer, display_texture, NULL, NULL);
  SDL_RenderPresent(display_renderer);
}

void display_cleanup() {
  SDL_DestroyWindow(display_window);
  SDL_DestroyRenderer(display_renderer);
  SDL_DestroyTexture(display_texture);
  SDL_Quit();
}

// Returns 0x0–0xF for valid CHIP-8 keys, or 0xFF if not mapped.
static uint8_t map_sdl_scancode(SDL_Scancode sc) {
  switch (sc) {
    case SDL_SCANCODE_1:
      return 0x1;
    case SDL_SCANCODE_2:
      return 0x2;
    case SDL_SCANCODE_3:
      return 0x3;
    case SDL_SCANCODE_4:
      return 0xC;
    case SDL_SCANCODE_Q:
      return 0x4;
    case SDL_SCANCODE_W:
      return 0x5;
    case SDL_SCANCODE_E:
      return 0x6;
    case SDL_SCANCODE_R:
      return 0xD;
    case SDL_SCANCODE_A:
      return 0x7;
    case SDL_SCANCODE_S:
      return 0x8;
    case SDL_SCANCODE_D:
      return 0x9;
    case SDL_SCANCODE_F:
      return 0xE;
    case SDL_SCANCODE_Z:
      return 0xA;
    case SDL_SCANCODE_X:
      return 0x0;
    case SDL_SCANCODE_C:
      return 0xB;
    case SDL_SCANCODE_V:
      return 0xF;
    default:
      return 0xFF;
  }
}
static void update_keyboard_state() {
  const uint8_t* state = SDL_GetKeyboardState(NULL);

  // Check all 16 CHIP-8 keys
  SDL_Scancode scancodes[] = {
      SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4,
      SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_R,
      SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_F,
      SDL_SCANCODE_Z, SDL_SCANCODE_X, SDL_SCANCODE_C, SDL_SCANCODE_V};

  for (int i = 0; i < 16; i++) {
    uint8_t chip8_key = map_sdl_scancode(scancodes[i]);
    if (state[scancodes[i]]) {
      chip8_key_down(chip8_key);
    } else {
      chip8_key_up(chip8_key);
    }
  }
}

bool display_poll_events() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      return true;
    }
  }

  update_keyboard_state();
  return false;
}

void display_render(const uint8_t* screen) {
  for (unsigned y = 0; y < SCREEN_H; y++) {
    for (unsigned x = 0; x < SCREEN_W; x++) {
      uint8_t pixel = screen[SCREEN_IDX(y, x)];

      pixels[SCREEN_IDX(y, x)] = pixel ? WHITE : BLACK;
    }
  }

  SDL_UpdateTexture(display_texture, NULL, pixels, SCREEN_W * sizeof(uint32_t));

  SDL_RenderClear(display_renderer);
  SDL_RenderCopy(display_renderer, display_texture, NULL, NULL);
  SDL_RenderPresent(display_renderer);
}