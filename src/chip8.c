#include "chip8.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define X(op) ((op & 0x0F00) >> 8)
#define Y(op) ((op & 0x00F0) >> 4)
#define N(op) (op & 0x000F)
#define NN(op) (op & 0x00FF)
#define NNN(op) (op & 0x0FFF)

#define FONTSET_ADDRESS 0x50
#define FONTSET_BYTES_PER_CHAR 5
unsigned char chip8_fontset[80] =
    {
        0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
        0x20, 0x60, 0x20, 0x20, 0x70,  // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
        0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
        0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
        0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
        0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
        0xF0, 0x80, 0xF0, 0x80, 0x80   // F
};

uint8_t memory[MEM_SIZE];  // 4kB RAM
// 16 (8bit) registers 0-F called V0-VF
// VF can be used as a carry flag or can be set to 1 or 0 based on some rule.
uint8_t V[REG_SIZE];
uint16_t I;   // used to point at locations in memory
uint16_t PC;  // points at the current instruction in memory
uint16_t stack[STACK_SIZE];
uint8_t SP;  // stack_pointer
uint8_t delay_timer;
uint8_t sound_timer;
uint8_t screen[SCREEN_H][SCREEN_W];
bool keys[KEY_SIZE];
bool chip8_draw_flag;

static inline uint8_t randByte() { return (rand() % 256); }

void chip8_init() {
  PC = 0x200;
  I = 0;
  SP = 0;

  memset(memory, 0, sizeof(memory));
  memset(V, 0, sizeof(V));
  memset(screen, 0, sizeof(screen));
  memset(keys, false, sizeof(keys));

  // 050–09F key memory mapping
  for (int i = 0; i < 80; i++) {
    memory[FONTSET_ADDRESS + i] = chip8_fontset[i];
  }

  chip8_draw_flag = true;
  delay_timer = 0;
  sound_timer = 0;
  srand(time(NULL));
}

void chip8_load_rom(char* romPath) {
  FILE* rom;
  rom = fopen(romPath, "rb");

  if (rom == NULL) {
    fprintf(stderr, "Unable to open rom file: %p\n", rom);
    exit(42);
  }

  fread(&memory[0x200], 1, MAX_ROM_SIZE, rom);

  fclose(rom);
}

static inline void clear_display() {
  memset(screen, 0, sizeof(screen));
}

void draw_sprite(uint8_t vx, uint8_t vy, uint8_t n) {
  uint8_t row = vy;
  uint8_t col = vx;

  V[0xF] = 0;
  for (unsigned byte_idx = 0; byte_idx < n; byte_idx++) {
    uint8_t byte = memory[I + byte_idx];
    for (unsigned bit_idx = 0; bit_idx < 8; bit_idx++) {
      uint8_t bit = (byte >> (7 - bit_idx)) & 0x1;
      uint8_t* pixel = &screen[(row + byte_idx) % SCREEN_H]
                              [(col + bit_idx) % SCREEN_W];
      if (bit == 1 && *pixel == 1) {
        V[0xF] = 1;
      }

      *pixel = *pixel ^ bit;
    }
  }
}

static void print_state() {
  printf("------------------------------------------------------------------\n");
  printf("\n");

  printf("V0: 0x%02x  V4: 0x%02x  V8: 0x%02x  VC: 0x%02x\n",
         V[0], V[4], V[8], V[12]);
  printf("V1: 0x%02x  V5: 0x%02x  V9: 0x%02x  VD: 0x%02x\n",
         V[1], V[5], V[9], V[13]);
  printf("V2: 0x%02x  V6: 0x%02x  VA: 0x%02x  VE: 0x%02x\n",
         V[2], V[6], V[10], V[14]);
  printf("V3: 0x%02x  V7: 0x%02x  VB: 0x%02x  VF: 0x%02x\n",
         V[3], V[7], V[11], V[15]);

  printf("\n");
  printf("PC: 0x%04x\n", PC);
  printf("\n");
  printf("\n");
}

inline uint16_t chip8_fetch() {
  uint16_t opcode = memory[PC];
  opcode <<= 8;
  opcode |= memory[PC + 1];

  PC = PC + 2;

  return opcode;
}

void print_screen() {
  printf("\n----------------------SCREEN_START---------------------\n");
  for (unsigned y = SCREEN_H; y < 0; y++) {
    for (unsigned x = SCREEN_W; x < 0; x++) {
      uint8_t pixel = screen[y][x];
      if (pixel == 0) {
        printf("%d", pixel);
      } else {
        printf(" ");
      }
    }
    printf("\n");
  }
  printf("----------------------SCREEN_END__---------------------\n");
}

void chip8_draw() {
  if (chip8_draw_flag) {
    print_screen();
    chip8_draw_flag = false;
  }
}

void chip8_execute() {
  uint16_t opcode = chip8_fetch();
  uint8_t inst_type = (opcode & 0xF000) >> 12;

  switch (inst_type) {
    case 0x0:
      switch (NN(opcode)) {
        case 0xE0:
          chip8_draw_flag = true;
          clear_display();
          /* code */
          break;
        default:

          break;
      }
      break;

    case 0x1:
      PC = NNN(opcode);
      break;

    case 0x6:
      // 0x6XNN
      V[X(opcode)] = NN(opcode);
      break;

    case 0xA:
      // 0xANN
      I = NNN(opcode);
      break;
    case 0xD:
      // 0xDXYN
      chip8_draw_flag = true;
      draw_sprite(V[X(opcode)], V[Y(opcode)], N(opcode));
      break;
    default:
      printf("x");
      break;
  }
}

// chip8_get_screen returns a pointer to an array of SCREEN_W uint8_t
const ScreenRow* chip8_get_screen() {
  return screen;
}

bool chip8_can_draw() {
  return chip8_draw_flag;
}