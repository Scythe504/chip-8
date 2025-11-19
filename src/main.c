#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

#include "chip8.h"
#include "display.h"

#define DISPLAY_USEC 16667  // ~60 Hz
#define CPU_USEC 1428       // ~700 Hz
#define USEC 1000000LL

// return microseconds difference curr - prev
int64_t timediff_usec(const struct timeval* prev, const struct timeval* curr) {
  int64_t sec_diff = (int64_t)curr->tv_sec - (int64_t)prev->tv_sec;
  int64_t usec_diff = (int64_t)curr->tv_usec - (int64_t)prev->tv_usec;
  return sec_diff * USEC + usec_diff;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <path/to/your/rom>\n", argv[0]);
    return 42;
  }

  chip8_init();
  display_init();
  chip8_load_rom(argv[1]);

  struct timeval cpu_clock_prev;
  struct timeval display_clock_prev;
  struct timeval now;

  gettimeofday(&cpu_clock_prev, NULL);
  gettimeofday(&display_clock_prev, NULL);

  while (!display_poll_events()) {
    gettimeofday(&now, NULL);

    // CPU cycles: run one step when elapsed >= CPU_USEC
    if (timediff_usec(&cpu_clock_prev, &now) >= CPU_USEC) {
      chip8_execute();
      // advance the cpu_clock_prev to now to avoid re-executing immediately
      // (simple approach; for more accurate timing you can advance by multiples of CPU_USEC)
      cpu_clock_prev = now;
    }

    // Timers tick at ~60Hz
    gettimeofday(&now, NULL);
    if (timediff_usec(&display_clock_prev, &now) >= DISPLAY_USEC) {
      chip8_tick();
      display_clock_prev = now;
    }

    // Render when emulator requests it
    if (chip8_can_draw()) {
      // printf("DRAWING\n");
      display_render(chip8_get_screen());
      chip8_set_draw_false();
    }
  }

  display_cleanup();
  return 0;
}
