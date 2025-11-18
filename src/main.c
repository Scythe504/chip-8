#include <stdio.h>
#include "chip8.h"
#include "display.h"


int main(int argc, char **argv) {
  if (argc != 2) {
    perror("Usage: ./chip8 <path/to/your/rom>");
    return 42;
  }

  chip8_init();
  display_init();
  char *romPath = argv[1];

  chip8_load_rom(romPath);

  display_loop();

  display_cleanup();
  // while (true)
  // {
  //   /* code */
    // chip8_execute();

  //   chip8_draw();
  // }


  return 0;
}