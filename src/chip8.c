#include "chip8.h"
#include "audio.h"

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
uint8_t screen[SCREEN_SIZE];
bool keys[KEY_SIZE];
bool chip8_draw_flag;

// Debug write helper: detects writes into ROM area and specific addresses
static void debug_mem_write(uint16_t addr, uint8_t value, const char* why) {
  // detect writes to ROM region (0x200–0xFFF where ROM lives)
  if (addr < 0x200) {
    fprintf(stderr, "WARN: memory write to ROM[0x%03X] = 0x%02X (%s)\n", addr, value, why);
  }

  // detect writes to the problematic loop area
  if (addr == 0x3DC || addr == 0x3DE) {
    fprintf(stderr, "TRACE: write to 0x%03X = 0x%02X (%s)\n", addr, value, why);
  }

  memory[addr] = value;
}

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

  chip8_draw_flag = false;
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

      // Calculate pixel position with wrapping
      int screen_y = (row + byte_idx) % SCREEN_H;
      int screen_x = (col + bit_idx) % SCREEN_W;

      // Bounds check
      if (screen_y < 0 || screen_y >= SCREEN_H || screen_x < 0 || screen_x >= SCREEN_W) {
        printf("DRAW_SPRITE OOB: y=%d, x=%d (SCREEN_H=%d, SCREEN_W=%d)\n",
               screen_y, screen_x, SCREEN_H, SCREEN_W);
        continue;
      }

      uint8_t* pixel = &screen[screen_y * SCREEN_W + screen_x];
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

uint16_t chip8_fetch() {
  uint16_t opcode = memory[PC];
  opcode <<= 8;
  opcode |= memory[PC + 1];

  PC = PC + 2;

  return opcode;
}

void debug_dump_screen() {
  static int frame = 0;
  char filename[64];
  snprintf(filename, sizeof(filename), "frame_%04d.txt", frame++);

  FILE* f = fopen(filename, "w");
  for (int y = 0; y < SCREEN_H; y++) {
    for (int x = 0; x < SCREEN_W; x++) {
      fprintf(f, "%c", screen[SCREEN_IDX(y, x)] ? '#' : '.');
    }
    fprintf(f, "\n");
  }
  fclose(f);
}

void chip8_draw() {
  if (chip8_draw_flag) {
    debug_dump_screen();
    chip8_draw_flag = false;
  }
}

void chip8_key_down(uint8_t key) {
  if (key < 16)
    keys[key] = true;
}

void chip8_key_up(uint8_t key) {
  if (key < 16) {
    keys[key] = false;
  }
}

void chip8_execute() {
  uint16_t opcode = chip8_fetch();
  uint8_t inst_type = (opcode & 0xF000) >> 12;
  // printf("PC=0x%04X OPCODE=0x%04X\n", PC, opcode);
  if (SP > STACK_SIZE) {
    fprintf(stderr, "SP overflow %u\n", SP);
    exit(1);
  }
  if (I >= MEM_SIZE) {
    fprintf(stderr, "I OOB: 0x%X\n", I);
    exit(1);
  }
  if (PC >= MEM_SIZE) {
    fprintf(stderr, "PC OOB: 0x%X\n", PC);
    exit(1);
  }

  switch (inst_type) {
    case 0x0: {
      switch (NN(opcode)) {
        case 0xE0:
          // clears the screen
          clear_display();
          chip8_draw_flag = true;
          break;
        case 0xEE: {
          if (SP == 0) {
            fprintf(stderr, "Stack underflow on 00EE\n");
            exit(1);
          }
          PC = stack[--SP];
          break;
        }
        default:
          // do nothing for 0NNN
          break;
      }
      break;
    }
    case 0x1: {
      PC = NNN(opcode);
      break;
    }
    case 0x2: {
      // Calls subroutine at NNN
      stack[SP++] = PC;
      PC = NNN(opcode);
      break;
    }
    case 0x3: {
      if (V[X(opcode)] == NN(opcode)) PC += 2;
      break;
    }
    case 0x4: {
      if (V[X(opcode)] != NN(opcode)) PC += 2;
      break;
    }
    case 0x5: {
      if (V[X(opcode)] == V[Y(opcode)]) PC += 2;
      break;
    }
    case 0x6: {
      // 0x6XNN
      V[X(opcode)] = NN(opcode);
      break;
    }
    case 0x7: {
      // Adds NN to the VX (carry flag is not changed)
      V[X(opcode)] += NN(opcode);
    } break;
    case 0x8: {
      switch (N(opcode)) {
        case 0x0: {  // LD Vx, Vy
          V[X(opcode)] = V[Y(opcode)];
          break;
        }

        case 0x1: {  // OR
          V[X(opcode)] |= V[Y(opcode)];
          break;
        }

        case 0x2: {  // AND
          V[X(opcode)] &= V[Y(opcode)];
          break;
        }

        case 0x3: {  // XOR
          V[X(opcode)] ^= V[Y(opcode)];
          break;
        }

        case 0x4: {  // ADD Vx, Vy (with carry)
          uint16_t sum = V[X(opcode)] + V[Y(opcode)];
          V[0xF] = (sum > 0xFF);
          V[X(opcode)] = sum & 0xFF;
          break;
        }

        case 0x5: {  // SUB Vx -= Vy
          uint8_t vx = V[X(opcode)];
          uint8_t vy = V[Y(opcode)];
          V[0xF] = (vx >= vy);
          V[X(opcode)] = vx - vy;
          break;
        }

        case 0x6: {  // SHR Vx
          uint8_t vx = V[X(opcode)];
          V[0xF] = vx & 0x01;  // LSB
          V[X(opcode)] = vx >> 1;
          break;
        }

        case 0x7: {  // SUBN Vx = Vy - Vx
          uint8_t vx = V[X(opcode)];
          uint8_t vy = V[Y(opcode)];
          V[0xF] = (vy >= vx);  // no borrow
          V[X(opcode)] = vy - vx;
          break;
        }

        case 0xE: {  // SHL Vx
          uint8_t vx = V[X(opcode)];
          V[0xF] = (vx & 0x80) >> 7;  // MSB before shift
          V[X(opcode)] = vx << 1;
          break;
        }
      }
      break;
    }
    case 0x9: {                      // 9XY0 — skip if VX != VY
      if ((opcode & 0x000F) == 0) {  // Only valid if last nibble = 0
        if (V[X(opcode)] != V[Y(opcode)]) {
          PC += 2;
        }
      }
      break;
    }

    case 0xA: {
      // 0xANN
      I = NNN(opcode);
      break;
    }
    case 0xB: {
      // Ambiguous could be PC=V0 + NNN or PC=VX + NNN
      PC = V[X(opcode)] + NNN(opcode);
      break;
    }
    case 0xC: {
      V[X(opcode)] = randByte() & NN(opcode);
      break;
    }
    case 0xD: {
      // 0xDXYN
      chip8_draw_flag = true;
      draw_sprite(V[X(opcode)] % SCREEN_W, V[Y(opcode)] % SCREEN_H, N(opcode));
      break;
    }
    case 0xE: {
      uint8_t vx = V[X(opcode)] & 0xF;
      switch (NN(opcode)) {
        case 0x9E: {
          if (keys[vx]) PC += 2;
          break;
        }
        case 0xA1: {
          if (!keys[vx]) PC += 2;
          break;
        }
      }
      break;
    }
    case 0xF: {
      switch (NN(opcode)) {
        case 0x07: {
          V[X(opcode)] = delay_timer;
          break;
        }

        case 0x0A: {
          uint8_t x = X(opcode);
          bool key_pressed = false;

          for (int k = 0; k < KEY_SIZE; k++) {
            if (keys[k]) {
              V[x] = k;
              key_pressed = true;
              break;
            }
          }

          if (!key_pressed) {
            PC -= 2;
          }

          break;
        }
        case 0x15: {
          delay_timer = V[X(opcode)];
          break;
        }
        case 0x18: {
          sound_timer = V[X(opcode)];
          break;
        }
        case 0x1E: {
          I += V[X(opcode)];
          break;
        }
        case 0x29: {
          I = FONTSET_ADDRESS + V[X(opcode)] * FONTSET_BYTES_PER_CHAR;
          break;
        }
        case 0x33: {
          uint8_t vx = V[X(opcode)];
          memory[I] = vx / 100;
          memory[I + 1] = (vx / 10) % 10;
          memory[I + 2] = vx % 10;

          break;
        }
        case 0x55: {  // LD [I], V0..VX  (then I += X + 1)
          uint8_t x = X(opcode);
          for (unsigned idx = 0; idx <= x; idx++) {
            // safety check:
            if ((size_t)I + idx >= MEM_SIZE) {
              fprintf(stderr, "FX55 write OOB I+%u = 0x%X\n", idx, I + idx);
              exit(1);
            }
            memory[I + idx] = V[idx];
          }
          I = I + x + 1;
          break;
        }

        case 0x65: {  // LD V0..VX, [I] (then I += X + 1)
          uint8_t x = X(opcode);
          for (unsigned idx = 0; idx <= x; idx++) {
            // if ((size_t)I + idx >= MEM_SIZE) {
            //   fprintf(stderr, "FX65 read OOB I+%u = 0x%X\n", idx, I + idx);
            //   exit(1);
            // }
            V[idx] = memory[I + idx];
          }
          I = I + x + 1;
          break;
        }
      }
      break;
    }
    default: {
      printf("unknown_opcode: 0x%04X\n", opcode);
      break;
    }
  }

  // print_state();
}

// sets the display and sound timers
void chip8_tick() {
  if (delay_timer > 0) --delay_timer;
  if (sound_timer > 0) {
    --sound_timer;
    audio_beep_on();
  } else {
    audio_beep_off();
  }
}

// chip8_get_screen returns a pointer to an array of SCREEN_W uint8_t
const uint8_t* chip8_get_screen() {
  return screen;
}

bool chip8_can_draw() {
  return chip8_draw_flag;
}

void chip8_set_draw_false() {
  chip8_draw_flag = false;
}