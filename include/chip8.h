#ifndef __CHIP_8_H__
#define __CHIP_8_H__
#include <stdint.h>
#include <stdbool.h>

#define MEM_SIZE 4096
#define SCREEN_H 32
#define SCREEN_W 64
#define REG_SIZE 16
#define SCREEN_SIZE (SCREEN_H * SCREEN_W)
#define STACK_SIZE 16
#define KEY_SIZE 16
#define MAX_ROM_SIZE (0x1000 - 0x200)
#define SCREEN_IDX(row, col) ((row)*SCREEN_W + (col))

void chip8_init(); //initializes chip8 vars
uint16_t chip8_fetch();
void chip8_execute();
void chip8_load_rom(char *romPath); // loads rom from the given filepath
void chip8_setkeys();
void chip8_tick();
void chip8_draw();

#endif // __CHIP_8_H__