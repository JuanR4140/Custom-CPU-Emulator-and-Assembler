#include <stdio.h>
#include "emu/mem.h"
#include "emu/cpu.h"

int main(void) {
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
