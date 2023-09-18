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
    exec(&memory, &cpu);
  }
  return 0;
}
