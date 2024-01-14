#ifndef CPU_H
#define CPU_H

#include "mem.h"
#include <stdbool.h>

enum opcodes{
  /*INS_MOV_VAL = 0x00, // mov ax, 5
  INS_MOV_REG = 0x01, // mov ax, bx
  INS_MOV_ADR = 0x02, // mov ax, $100*/

  INS_MOV_REG_VAL = 0x00, // mov ax, 5
  INS_MOV_REG_REG = 0x01, // mov ax, bx
  INS_MOV_REG_PTR_REG = 0x02, // mov ax, [bx]
  INS_MOV_PTR_REG_REG = 0x03, // mov [bx], ax
  INS_MOV_REG_PTR_REG_OFFSET_VAL = 0x04, // mov ax, [bx + 5]
  INS_MOV_REG_PTR_REG_OFFSET_REG = 0x05, // mov ax, [bx + cx]
  INS_MOV_PTR_REG_OFFSET_VAL_REG = 0x06, // mov [bx + 5], ax
  INS_MOV_PTR_REG_OFFSET_REG_REG = 0x07, // mov [bx + cx], ax
  INS_MOV_REG_ADR = 0x08, // mov ax, [0x100]
  INS_MOV_ADR_REG = 0x09, // mov [0x100], ax

  INS_CMP_VAL = 0x20, // cmp ax, 5
  INS_CMP_REG = 0x21, // cmp ax, bx

  INS_JMP = 0x30, // jmp label
  INS_JE  = 0x31, // je label
  INS_JNE = 0x32, // jne label
  INS_JL  = 0x33, // jl label
  INS_JG  = 0x34, // jg label
  INS_CALL = 0x35, // call label
  INS_RET = 0x36, // ret

  INS_ADD_VAL = 0x40, // add ax, 5
  INS_ADD_REG = 0x41, // add ax, bx

  INS_SUB_VAL = 0x50, // sub ax, 5
  INS_SUB_REG = 0x51, // sub ax, bx

  INS_MUL_REG = 0x60, // mul cx = ax * cx -> dx:ax

  INS_DIV_REG = 0x70, // div bx = ax / bx -> dx r ax

  INS_PUSH_VAL = 0x80, // push 5
  INS_PUSH_REG = 0x81, // push ax

  INS_POP_REG = 0x90, // pop ax

  INS_AND_REG = 0xA0, // and ax, bx
  INS_NOT_REG = 0xA1, // not ax
  INS_OR_REG  = 0xA2, // or ax, bx
  INS_XOR_REG = 0xA3, // xor ax, bx

  INS_SHR_VAL = 0xB0, // shr ax, 5
  INS_SHR_REG = 0xB1, // shr ax, bx
  INS_SHL_VAL = 0xB2, // shl ax, 5
  INS_SHL_REG = 0xB3, // shl ax, bx

  INS_HLT = 0xFE, // hlt
  INS_NOP = 0xFF // nop
};

/*enum opcodes{
  INS_LDA_IM = 0x10
};*/

typedef struct CPU{
  Word PC; // Program Counter
  Word BP; // Base Pointer
  Word SP; // Stack Pointer
  Word ax, bx, cx, dx; // General purpose registers
  Byte ah, al, bh, bl, ch, cl, dh, dl;

  Byte C : 1; // Carry flag
  Byte Z : 1; // Zero flag
  Byte I : 1; // Interrupt flag
  Byte D : 1; // Decimal flag
  Byte B : 1; // Break flag
  Byte V : 1; // Overflow flag
  Byte N : 1; // Negative flag
} CPU;

void exec(Memory* memory, CPU* cpu, bool show_debug_info);
void initializeCPU(CPU* cpu);
void drawScreen(Memory* memory, CPU* cpu, bool show_debug_info);

#endif
