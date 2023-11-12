#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include "mem.h"
#include "cpu.h"

void throw_missing_file(char* file){
  endwin();
  printf("Error.\nCould not read program '%s', which is needed by CPU.\n", file);
  exit(1);
}

void load_program(char* program_name, int starting_address, Memory* memory){
  FILE* program;
  program = fopen(program_name, "rb");
  if(program == NULL) throw_missing_file(program_name);
  fseek(program, 0, SEEK_END);
  long program_size = ftell(program);
  fseek(program, 0, SEEK_SET);
  char program_buffer[program_size];
  int bytes_read = fread(program_buffer, 1, program_size, program);
  if(bytes_read < program_size){
    throw_missing_file(program_name);
  }

  int program_address = starting_address;
  for(int i = 0; i < program_size; i++){
    memory->data[program_address] = program_buffer[i];
    program_address++;
  }

  // I AM AN IDIOT I SHOULD NOT BE A PROGRAMMER.
  /*int program_index = 0;
  for(int i = starting_address; i < program_size; i++){
    memory->data[i] = program_buffer[program_index];
    program_index++;
  }*/
  // printf("Program index: %d\nProgram_size: %ld\n", program_index, program_size);
  // printf("program_index: %d\nRead %d bytes from %s.\n", program_index, bytes_read, program_name);
  // usleep(1000 * 1000);
  fclose(program);
}

void initializeMemory(Memory* memory){
  for(u32 i = 0; i < MAX; i++){
    memory->data[i] = 0;
  }

  // Set up vram
  for(u32 i = 0x300; i < 0x3C9; i++){
    memory->data[i] = 'X';
  }
  // memory->data[0x0] = INS_HLT;

  /*FILE* vram_prog;
  vram_prog = fopen("./emu/core/vram.out", "rb");
  if(vram_prog == NULL) throw_missing_file("./emu/core/vram.out");
  fseek(vram_prog, 0, SEEK_END);
  long vram_prog_size = ftell(vram_prog);
  fseek(vram_prog, 0, SEEK_SET);
  char vram[vram_prog_size];
  fread(vram, 1, vram_prog_size, vram_prog);
  
  for(u32 i = 0; i < vram_prog_size; i++){
    memory->data[i] = vram[i];
  }

  fclose(vram_prog);*/

  load_program("./emu/core/boot/vram.out", 0x0, memory);
  load_program("./emu/core/boot/jmp_to_code.out", 0x20, memory);
  load_program("./emu/slot/cartridge.out", 0xE000, memory);
  
  // Hardcode CPU instructions at 0?

  // Set up vram
  /*memory->data[0x0] = INS_MOV_VAL;
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

  memory->data[0x20] = INS_HLT;*/

}
