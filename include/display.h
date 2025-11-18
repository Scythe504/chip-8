#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <stdbool.h>
#include <stdint.h>

#include "chip8.h"

// Initialize SDL window and renderer/OpenGL context.
void display_init();

// Render the CHIP-8 framebuffer (screen[32][64]) to the window.
// This function should take the buffer as input eventually.
void display_render(const uint8_t screen[SCREEN_H][SCREEN_W]);

// Poll SDL events (keyboard + quit).
// Should return true when user requests quit.
bool display_poll_events();

// Destroy window and quit SDL.
void display_cleanup();

#endif
