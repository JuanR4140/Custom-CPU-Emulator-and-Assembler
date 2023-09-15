#ifndef MEM_H
#define MEM_H

#define Byte unsigned char
#define Word unsigned short
#define u32 unsigned int

#define MAX 1024 * 64
#define WIDTH 20
#define HEIGHT 10


typedef struct Memory{
  Byte data[MAX];
} Memory;

void initializeMemory(Memory* memory);

#endif