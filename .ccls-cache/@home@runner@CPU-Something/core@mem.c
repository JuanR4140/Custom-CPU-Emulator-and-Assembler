#include "mem.h"
#include "cpu.h"

void initializeMemory(Memory* memory){  for(u32 i = 0; i < MAX; i++){
    memory->data[i] = 0;
  }

  // Set up vram
  for(u32 i = 0x300; i < 0x3C9; i++){
    memory->data[i] = 'X';
  }
  
  // Hardcode CPU instructions at 0?

  // Set up vram
  memory->data[0x0] = INS_MOV_VAL;
  memory->data[0x1] = 0x0; // ax
  memory->data[0x2] = 0xC8; // lsb
  memory->data[0x3] = 0x00; // msb // 0x00C8

  memory->data[0x4] = INS_MOV_VAL;
  memory->data[0x5] = 0x1; // bx
  memory->data[0x6] = 0x00;
  memory->data[0x7] = 0x03; // 0x0300

  memory->data[0x8] = INS_MOV_VAL;
  memory->data[0x9] = 0x2; // cx
  memory->data[0xA] = 0x23; // '#'
  memory->data[0xB] = 0x00; // 0x0023

  memory->data[0xC] = INS_STR_REG;
  memory->data[0xD] = 0x2; // cx
  memory->data[0xE] = 0x1; // bx
  memory->data[0xF] = 0x0;

  memory->data[0x10] = INS_SUB_VAL;
  memory->data[0x11] = 0x0;
  memory->data[0x12] = 0x1;
  memory->data[0x13] = 0x0;

  memory->data[0x14] = INS_ADD_VAL;
  memory->data[0x15] = 0x1;
  memory->data[0x16] = 0x1;
  memory->data[0x17] = 0x0;

  memory->data[0x18] = INS_CMP_VAL;
  memory->data[0x19] = 0x0;
  memory->data[0x1A] = 0x0;
  memory->data[0x1B] = 0x0;

  memory->data[0x1C] = INS_JNE;
  memory->data[0x1D] = 0x0; // ignored
  memory->data[0x1E] = 0xC;
  memory->data[0x1F] = 0x0;

  memory->data[0x20] = INS_HLT;

}
