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

    case INS_MOV_REG:{
      Byte reg_dst = fetchByte(memory, cpu);
      Byte reg_src = fetchByte(memory, cpu);
      Byte unused = fetchByte(memory, cpu);

      *(registers[reg_dst]) = reg_src; 
      break;
    }

    case INS_MOV_ADR:{
      Byte reg = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);

      *(registers[reg]) = readWord(memory, val);
      break;
    }

    case INS_STR_REG:{
      Byte reg_src = fetchByte(memory, cpu);
      Byte reg_dst = fetchByte(memory, cpu);
      Byte unused = fetchByte(memory, cpu);

      writeWord(memory, *(registers[reg_dst]), *(registers[reg_src]));
      /*Byte reg_src = fetchByte(memory, cpu);
      Byte reg_dst = fetchByte(memory, cpu);
      Byte unused = fetchByte(memory, cpu);
      writeByte(memory, *(registers[reg_dst]), *(registers[reg_src]));*/
      break;
    }

    case INS_STH_REG:{
      Byte reg_src = fetchByte(memory, cpu);
      Byte reg_dst = fetchByte(memory, cpu);
      Byte unused = fetchByte(memory, cpu);

      Byte high_byte = (*(registers[reg_src]) >> 8) & 0xFF;
      writeByte(memory, *(registers[reg_dst]), high_byte);
      break;
    }

    case INS_STL_REG:{
      Byte reg_src = fetchByte(memory, cpu);
      Byte reg_dst = fetchByte(memory, cpu);
      Byte unused = fetchByte(memory, cpu);

      Byte low_byte = (*(registers[reg_src]) & 0xFF);
      writeByte(memory, *(registers[reg_dst]), low_byte);
      break;
    }

    case INS_STR_ADR:{
      Byte reg = fetchByte(memory, cpu);
      Word adr = fetchWord(memory, cpu);

      writeWord(memory, adr, *(registers[reg]));
      break;
    }

    case INS_STH_ADR:{
      Byte reg = fetchByte(memory, cpu);
      Word adr = fetchWord(memory, cpu);

      Byte high_byte = (*(registers[reg]) >> 8) & 0xFF;
      writeByte(memory, adr, high_byte);
      break;
    }

    case INS_STL_ADR:{
      Byte reg = fetchByte(memory, cpu);
      Word adr = fetchWord(memory, cpu);

      Byte low_byte = (*(registers[reg]) & 0xFF);
      writeByte(memory, adr, low_byte);
      break;
    }

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

    case INS_CMP_ADR:{
      Byte reg = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);

      cpu->Z = ( *(registers[reg]) - readWord(memory, val) ) == 0;
      cpu->N = ( *(registers[reg]) - readWord(memory, val) ) < 0;
      Word result = *(registers[reg]) - readWord(memory, val);
      cpu->V = ((*registers[reg] & 0x8000) != (result & 0x8000)) && ((*registers[reg] & 0x8000) != (result & 0x8000));
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

      cpu->SP--;
      writeWord(memory, cpu->SP, cpu->PC);
      cpu->PC = val;
      break;
    }
    // Will debug CALL and RET soon. Mostly RET.
    case INS_RET:{
      Byte unused = fetchByte(memory, cpu);
      Word val = fetchWord(memory, cpu);

      cpu->SP += val;
      Word adr = readWord(memory, cpu->SP);
      cpu->SP++;
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

      cpu->SP--;
      writeWord(memory, cpu->SP, val);
      break;
    }

    case INS_PUSH_REG:{
      Byte reg = fetchByte(memory, cpu);
      Word unused = fetchWord(memory, cpu);

      cpu->SP--;
      writeWord(memory, cpu->SP, *(registers[reg]));
      break;
    }

    case INS_POP_REG:{
      Byte reg = fetchByte(memory, cpu);
      Word unused = fetchWord(memory, cpu);

      *(registers[reg]) = readWord(memory, cpu->SP);
      cpu->SP++;
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
      for(;;);
      break;
    }

    case INS_NOP:{
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
