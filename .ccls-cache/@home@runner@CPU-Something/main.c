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
  //drawScreen(&memory);
  
  /*exec(&memory, &cpu);
  printf("value of ax: %i\n", cpu.ax);*/
  /*for(int i = 0; i < HEIGHT; i++){
    for(int j = 0; j < WIDTH; j++){
      printf("#");
    }
    printf("\n");
  }*/
  return 0;
}
