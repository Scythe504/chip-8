// src/main.c
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "audio.h"
#include "chip8.h"
#include "display.h"

// Timing configuration
#define CPU_HZ 1200
#define TIMER_HZ 60

static int64_t timediff_usec(const struct timeval* prev, const struct timeval* curr) {
  return ((int64_t)curr->tv_sec - (int64_t)prev->tv_sec) * 1000000LL +
         ((int64_t)curr->tv_usec - (int64_t)prev->tv_usec);
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <path/to/rom>\n", argv[0]);
    return 42;
  }

  chip8_init();
  display_init();
  audio_init();
  chip8_load_rom(argv[1]);

  const int64_t cpu_step = 1000000LL / CPU_HZ;
  const int64_t timer_step = 1000000LL / TIMER_HZ;

  int64_t cpu_acc = 0;
  int64_t timer_acc = 0;

  struct timeval last;
  gettimeofday(&last, NULL);

  bool quit = false;
  while (!quit) {
    if (display_poll_events())
      quit = true;

    struct timeval now;
    gettimeofday(&now, NULL);

    int64_t dt = timediff_usec(&last, &now);
    last = now;

    if (dt < 0) dt = 0;
    if (dt > 200000) dt = 200000;  // clamp to avoid death spiral

    cpu_acc += dt;
    timer_acc += dt;

    // --- CPU EXECUTION (multiple steps if needed) ---
    int steps = 0;
    const int MAX_STEPS = 1000;

    while (cpu_acc >= cpu_step && steps < MAX_STEPS) {
      chip8_execute();
      cpu_acc -= cpu_step;
      steps++;
    }

    // --- 60Hz Timers ---
    while (timer_acc >= timer_step) {
      chip8_tick();
      timer_acc -= timer_step;
    }

    // --- DRAW IF FLAGGED ---
    if (chip8_can_draw()) {
      display_render(chip8_get_screen());
      chip8_set_draw_false();
    }

    SDL_Delay(1);
  }

  display_cleanup();
  audio_cleanup();
  return 0;
}
