#include "cpu.h"
#include "mem.h"
#include <stdio.h>
#include <stdlib.h>

void drawScreen(Memory* memory);

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
  return memory->data[address];
}

void writeByte(Memory* memory, Word address, Byte value){
  memory->data[address] = value;
}

void writeWord(Memory* memory, Word address, Word value){
  memory->data[address] = value;
}

void LDASetStatus(CPU* cpu){
  cpu->Z = (cpu->ax == 0);
  cpu->N = (cpu->ax & 0b10000000) > 0;
}

void exec(Memory* memory, CPU* cpu){

  Word* registers[] = {&cpu->ax, &cpu->bx, &cpu->cx, &cpu->dx};
  
  Byte ins = fetchByte(memory, cpu);
  switch(ins){
    
    case INS_MOV_VAL:{
      Byte reg = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);
      *(registers[reg]) = val;
      // printf("Value of %X: %X\n", reg, *(registers[reg]));
      break;
    }

    case INS_STR_REG:{
      Byte reg_src = fetchByte(memory, cpu);
      Byte reg_dst = fetchByte(memory, cpu);
      Byte unused = fetchByte(memory, cpu);
      // *(registers[reg_dst]) = *(registers[reg_src]);
      writeByte(memory, *(registers[reg_dst]), *(registers[reg_src]));
      // printf("Writing %X to mem %X..\n", reg_src, *(registers[reg_dst]));
      // str ax, bx
      break;
    }

    case INS_ADD_VAL:{
      Byte reg = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);
      *(registers[reg]) += val;
      break;
    }

    case INS_SUB_VAL:{
      Byte reg = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);
      *(registers[reg]) -= val;
      break;
    }

    case INS_CMP_VAL:{
      Byte reg = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);
      cpu->Z = ( *registers[reg] - val ) == 0;
      break;
    }

    case INS_JNE:{
      Byte unused = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);
      if(!cpu->Z) cpu->PC = val;
      break;
    }

    case INS_HLT:{
      printf("HLT !!\n");
      drawScreen(memory);
      exit(0);
      break;
    }
    
  }

  /*switch(memory->data[cpu->PC]){
    case INS_MOV_VAL:{
      Byte reg = memory->data[cpu->PC+1];
      Word val = (memory->data[cpu->PC+3] << 8) | memory->data[cpu->PC+2];
      // printf("MACHINE CODE: %X %X %X\n", INS_MOV_VAL, reg, val);
      switch(reg){
        case 0x0:{ // ax
          cpu->ax = val;
          break;
        }
        case 0x1:{
          cpu->bx = val;
          break;
        }
        case 0x2:{
          cpu->cx = val;
          break;
        }
        case 0x3:{
          cpu->dx = val;
          break;
        }
      }
      break;
    }
  }*/
  
}

void drawScreen(Memory* memory){
  Word vram = 0x300;
  for(u32 i = 0; i < HEIGHT; i++){
    for(int j = 0; j < WIDTH; j++){
      printf("%c", memory->data[vram]);
      vram++;
    }
    printf("\n");
  }
}

void initializeCPU(CPU* cpu){
  cpu->ax = cpu->bx = cpu->cx = cpu->dx = 0;
  cpu->ah = cpu->al = cpu->bh = cpu->bl = cpu->ch = cpu->cl = cpu->dh = cpu->dl = 0;
  cpu->PC = 0x0;
  cpu->SP = 0x100;
  cpu->C = cpu->Z = cpu->I = cpu->D = cpu->B = cpu->V = cpu->N = 0;
}
