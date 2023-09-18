#include <stdio.h>
#include <ncurses.h>

#include "emu/mem.h"
#include "emu/cpu.h"

int main(void) {
  initscr();
  noecho();
  nodelay(stdscr, TRUE);
  
  CPU cpu;
  Memory memory;
  initializeCPU(&cpu);
  initializeMemory(&memory);
  cpu.ax = 0;
  for(;;){
    memory.data[0x3CB] = 0x0;
    int ch = getchar();
    if(ch != ERR){
      memory.data[0x3CB] = 0x1;
      memory.data[0x3CA] = ch;
    }
    
    exec(&memory, &cpu);
  }
  return 0;
}
