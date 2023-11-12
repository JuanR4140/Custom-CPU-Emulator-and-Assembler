#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>

#include "emu/mem.h"
#include "emu/cpu.h"

int main(int argc, char** argv) {
  bool show_debug_info = false;
  if(argc == 2){
    if(strcmp(argv[1], "-d") == 0){
      show_debug_info = true;
    }
  }
  initscr();
  noecho();
  nodelay(stdscr, TRUE);
  
  CPU cpu;
  Memory memory;
  initializeCPU(&cpu);
  initializeMemory(&memory);
  cpu.ax = 0;
  int ch;
  for(;;){
    
    ch = getch();
    if(ch != ERR && memory.data[0x3CB] == 0x0){
      memory.data[0x3CB] = 0x1;
      memory.data[0x3CA] = ch;
    }

    drawScreen(&memory, &cpu, show_debug_info);
    
    exec(&memory, &cpu);
  }

  echo();
  nodelay(stdscr, FALSE);
  endwin();
  return 0;
}
