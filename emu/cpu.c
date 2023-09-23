#include "cpu.h"
#include "mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>

void drawScreen(Memory* memory, CPU* cpu);

Byte fetchByte(Memory* memory, CPU* cpu){
  Byte data = memory->data[cpu->PC];
  cpu->PC++;
  return data;
}

Word fetchWord(Memory* memory, CPU* cpu){
  Word data = memory->data[cpu->PC];
  cpu->PC++;
  data |= (memory->data[cpu->PC] << 8);
  cpu->PC++;
  return data;
}

Byte readByte(Memory* memory, Byte address){
  return memory->data[address];
}

Word readWord(Memory* memory, Word address){
  return (memory->data[address] | (memory->data[address + 1] << 8));
}

void writeByte(Memory* memory, Word address, Byte value){
  memory->data[address] = value;
}

void writeWord(Memory* memory, Word address, Word value){
  memory->data[address] = (Byte)value;
  memory->data[address + 1] = (Byte)(value >> 8);
}

void LDASetStatus(CPU* cpu){
  cpu->Z = (cpu->ax == 0);
  cpu->N = (cpu->ax & 0b10000000) > 0;
}

void exec(Memory* memory, CPU* cpu){

  Word* registers[] = {&cpu->ax, &cpu->bx, &cpu->cx, &cpu->dx, &cpu->BP, &cpu->SP};
  
  Byte ins = fetchByte(memory, cpu);
  switch(ins){

    case INS_MOV_REG_VAL:{
      // mov ax, 5
      Byte reg = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);

      *(registers[reg]) = val;
      break;
    }

    case INS_MOV_REG_REG:{
      // mov ax, bx
      Byte reg_dst = fetchByte(memory, cpu);
      Byte reg_src = fetchByte(memory, cpu);
      Byte unused = fetchByte(memory, cpu);

      *(registers[reg_dst]) = *(registers[reg_src]);
      break;
    }

    case INS_MOV_REG_PTR_REG:{
      // mov ax, [bx]
      Byte reg_dst = fetchByte(memory, cpu);
      Byte reg_src = fetchByte(memory, cpu);
      Byte unused = fetchByte(memory, cpu);

      *(registers[reg_dst]) = readWord(memory, *(registers[reg_src]));
      break;
    }

    case INS_MOV_PTR_REG_REG:{
      // mov [bx], ax
      Byte reg_dst = fetchByte(memory, cpu);
      Byte reg_src = fetchByte(memory, cpu);
      Byte unused = fetchByte(memory, cpu);

      writeWord(memory, *(registers[reg_dst]), *(registers[reg_src]));
      break;
    }

    case INS_MOV_REG_PTR_REG_OFFSET_VAL:{
      // mov ax, [bx + 5]
      Byte reg_dst = fetchByte(memory, cpu);
      Byte reg_src = fetchByte(memory, cpu);
      Byte offset = fetchByte(memory, cpu);

      *(registers[reg_dst]) = readWord(memory, *(registers[reg_src]) + offset);
      break;
    }

    case INS_MOV_REG_PTR_REG_OFFSET_REG:{
      // mov ax, [bx + cx]
      Byte reg_dst = fetchByte(memory, cpu);
      Byte reg_src = fetchByte(memory, cpu);
      Byte reg_off = fetchByte(memory, cpu);

      *(registers[reg_dst]) = readWord(memory, *(registers[reg_src]) + *(registers[reg_off]));
      break;
    }

    case INS_MOV_PTR_REG_OFFSET_VAL_REG:{
      // mov [bx + 5], ax
      Byte reg_dst = fetchByte(memory, cpu);
      Byte offset = fetchByte(memory, cpu);
      Byte reg_src = fetchByte(memory, cpu);

      writeWord(memory, *(registers[reg_dst]) + offset, *(registers[reg_src]));
      break;
    }

    case INS_MOV_PTR_REG_OFFSET_REG_REG:{
      // mov [bx + cx], ax
      Byte reg_dst = fetchByte(memory, cpu);
      Byte reg_off = fetchByte(memory, cpu);
      Byte reg_src = fetchByte(memory, cpu);

      writeWord(memory, *(registers[reg_dst]) + *(registers[reg_off]), *(registers[reg_src]));
      break;
    }

    case INS_MOV_REG_ADR:{
      // mov ax, [0x100]
      Byte reg = fetchByte(memory, cpu);
      Word adr = fetchWord(memory, cpu);

      *(registers[reg]) = readWord(memory, adr);
      break;
    }

    case INS_MOV_ADR_REG:{
      // mov [0x100], ax
      Word adr = fetchWord(memory, cpu);
      Byte reg = fetchByte(memory, cpu);

      writeWord(memory, adr, *(registers[reg]));
      break;
    }
    
    /*case INS_MOV_VAL:{
      Byte reg = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);

      *(registers[reg]) = val;
      // printf("Value of %X: %X\n", reg, *(registers[reg]));
      break;
    }

    case INS_MOV_REG:{
      Byte reg_dst = fetchByte(memory, cpu);
      Byte reg_src = fetchByte(memory, cpu);
      Byte unused = fetchByte(memory, cpu);

      // OH MY GOD?
      // *(registers[reg_dst]) = reg_src;

      *(registers[reg_dst]) = *(registers[reg_src]);
      break;
    }

    case INS_MOV_ADR:{
      Byte reg = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);

      *(registers[reg]) = readWord(memory, val);
      break;
    }*/

    case INS_CMP_VAL:{
      Byte reg = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);

      cpu->Z = ( *registers[reg] - val ) == 0;
      cpu->N = ( *registers[reg] - val ) < 0;
      Word result = *registers[reg] - val;
      cpu->V = ((*registers[reg] & 0x8000) != (val & 0x8000)) && ((*registers[reg] & 0x8000) != (result & 0x8000));

      break;
    }

    case INS_CMP_REG:{
      Byte reg_dst = fetchByte(memory, cpu);
      Byte reg_src = fetchByte(memory, cpu);
      Byte unused = fetchByte(memory, cpu);

      cpu->Z = ( *(registers[reg_dst]) - *(registers[reg_src]) ) == 0;
      cpu->N = ( *(registers[reg_dst]) - *(registers[reg_src]) ) < 0;
      Word result = *(registers[reg_dst]) - *(registers[reg_src]);
      cpu->V = ((*registers[reg_dst] & 0x8000) != (result & 0x8000)) && ((*registers[reg_dst] & 0x8000) != (result & 0x8000));
      break;
    }

    case INS_JMP:{
      Byte unused = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);

      cpu->PC = val;
      break;
    }

    case INS_JE:{
      Byte unused = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);
      if(cpu->Z) cpu->PC = val;
      break;
    }

    case INS_JNE:{
      Byte unused = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);
      if(!cpu->Z) cpu->PC = val;
      break;
    }

    case INS_JL:{
      Byte unused = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);

      if(cpu->N && !cpu->Z) cpu->PC = val;
      break;
    }

    case INS_JG:{
      Byte unused = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);

      if(!cpu->N && !cpu->Z) cpu->PC = val;
      break;
    }

    case INS_CALL:{
      Byte unused = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);

      cpu->SP -= 2;
      writeWord(memory, cpu->SP, cpu->PC);
      cpu->PC = val;
      break;
    }
    // Will debug CALL and RET soon. Mostly RET.
    case INS_RET:{
      Byte unused = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);

      Word adr = readWord(memory, cpu->SP);
      cpu->SP += 2;
      cpu->SP += val;
      cpu->PC = adr;
      break;
    }

    case INS_ADD_VAL:{
      Byte reg = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);
      *(registers[reg]) += val;
      break;
    }

    case INS_ADD_REG:{
      Byte reg = fetchByte(memory, cpu);
      Byte reg2 = fetchByte(memory, cpu);
      Byte unused = fetchByte(memory, cpu);
      *(registers[reg]) += *(registers[reg2]);
      break;
    }

    case INS_SUB_VAL:{
      Byte reg = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);
      *(registers[reg]) -= val;
      break;
    }

    case INS_SUB_REG:{
      Byte reg = fetchByte(memory, cpu);
      Byte reg2 = fetchByte(memory, cpu);
      Byte unused = fetchByte(memory, cpu);
      *(registers[reg]) -= *(registers[reg2]);
      break;
    }

    case INS_MUL_REG:{
      Byte reg = fetchByte(memory, cpu);
      Word unused = fetchWord(memory, cpu);

      int result = cpu->ax * *(registers[reg]);

      cpu->dx = (result >> 16) & 0xFFFF;
      cpu->ax = (result & 0xFFFF);
      break;
    }

    case INS_DIV_REG:{
      Byte reg = fetchByte(memory, cpu);
      Word unused = fetchWord(memory, cpu);

      cpu->dx = cpu->ax / *(registers[reg]);
      cpu->ax = cpu->ax % *(registers[reg]);
      break;
    }

    case INS_PUSH_VAL:{
      Byte unused = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);

      cpu->SP -= 2;
      writeWord(memory, cpu->SP, val);
      break;
    }

    case INS_PUSH_REG:{
      Byte reg = fetchByte(memory, cpu);
      Word unused = fetchWord(memory, cpu);

      cpu->SP -= 2;
      writeWord(memory, cpu->SP, *(registers[reg]));
      break;
    }

    case INS_POP_REG:{
      Byte reg = fetchByte(memory, cpu);
      Word unused = fetchWord(memory, cpu);

      *(registers[reg]) = readWord(memory, cpu->SP);
      cpu->SP += 2;
      break;
    }

    case INS_AND_REG:{
      Byte reg = fetchByte(memory, cpu);
      Byte reg2 = fetchByte(memory, cpu);
      Byte unused = fetchByte(memory, cpu);

      *(registers[reg]) &= *(registers[reg2]);
      break;
    }

    case INS_NOT_REG:{
      Byte reg = fetchByte(memory, cpu);
      Word unused = fetchWord(memory, cpu);

      *(registers[reg]) = ~*(registers[reg]);
      break;
    }

    case INS_OR_REG:{
      Byte reg = fetchByte(memory, cpu);
      Byte reg2 = fetchByte(memory, cpu);
      Byte unused = fetchByte(memory, cpu);

      *(registers[reg]) |= *(registers[reg2]);
      break;
    }

    case INS_XOR_REG:{
      Byte reg = fetchByte(memory, cpu);
      Byte reg2 = fetchByte(memory, cpu);
      Byte unused = fetchByte(memory, cpu);

      *(registers[reg]) ^= *(registers[reg2]);
      break;
    }

    case INS_SHR_VAL:{
      Byte reg = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);

      *(registers[reg]) >>= val;
      break;
    }

    case INS_SHR_REG:{
      Byte reg = fetchByte(memory, cpu);
      Byte reg2 = fetchByte(memory, cpu);
      Byte unused = fetchByte(memory, cpu);

      *(registers[reg]) >>= *(registers[reg2]);
      break;
    }

    case INS_SHL_VAL:{
      Byte reg = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);

      *(registers[reg]) <<= val;
      break;
    }

    case INS_SHL_REG:{
      Byte reg = fetchByte(memory, cpu);
      Byte reg2 = fetchByte(memory, cpu);
      Byte unused = fetchByte(memory, cpu);

      *(registers[reg]) <<= *(registers[reg2]);
      break;
    }

    case INS_HLT:{
      endwin();
      drawScreen(memory, cpu);
      exit(0);
      break;
    }

    case INS_NOP:{
      Byte unused1 = fetchByte(memory, cpu);
      Word unused2 = fetchWord(memory, cpu);
      break;
    }
    
  }
  
}

void drawScreen(Memory* memory, CPU* cpu){
  endwin();
  system("clear");
  printf("PC: 0x%X\n", cpu->PC);
  printf("SP: 0x%X ('%c')\n", cpu->SP, memory->data[cpu->SP]);
  printf("BP: 0x%X ('%c')\n", cpu->BP, memory->data[cpu->BP]);
  printf("AX: 0x%X\n", cpu->ax);
  printf("BX: 0x%X\n", cpu->bx);
  printf("CX: 0x%X\n", cpu->cx);
  printf("DX: 0x%X\n", cpu->dx);
  Word vram = 0x300;
  for(u32 i = 0; i < HEIGHT; i++){
    for(int j = 0; j < WIDTH; j++){
      printf("%c", memory->data[vram]);
      vram++;
    }
    printf("\n");
  }
  initscr();
}

void initializeCPU(CPU* cpu){
  cpu->ax = cpu->bx = cpu->cx = cpu->dx = 0;
  cpu->ah = cpu->al = cpu->bh = cpu->bl = cpu->ch = cpu->cl = cpu->dh = cpu->dl = 0;
  cpu->PC = 0x0;
  cpu->SP = 0x200;
  cpu->C = cpu->Z = cpu->I = cpu->D = cpu->B = cpu->V = cpu->N = 0;
}
