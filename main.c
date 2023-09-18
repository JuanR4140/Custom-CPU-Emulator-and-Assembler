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

    /*
      This approach might not actually work! If a user
      presses a key and 0x1 gets written, the CPU might
      be executing some other instruction (JMP, CMP) that
      won't give the program enough time to CMP the value
      to get a new key, as the next cycle will immediately
      set it back to 0! 

      I propose 0x3CB stays on 0x1 and it's the programmer's
      job to write 0x0 back to it to "acknowledge" the read key.
    */
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
