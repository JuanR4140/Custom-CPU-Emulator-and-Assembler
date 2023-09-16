# Documentation for the Custom CPU's instruction set

## Table of Contents

- [About](#about)
- [General assembling](#general-assembling)
- [Assembling for CPU]()
- [MOV](#mov)
- [STR](#str)
- [CMP](#cmp)
- [JUMPING](#jumping)
- [ARITHMETIC](#arithmetic)
- [STACK](#stack)
- [BITWISE](#bitwise)
- [OTHER](#other)

## About

This documentation will go over how to assemble a program to be ran on the Custom CPU. It will also
go over all of the instruction set of the Custom CPU, ordered by type of instruction.

## General assembling

There is no easy support for strings or variables, so you'll have to write to memory locations directly!

To assemble a program, first compile the project with `make` on the root folder so the executable is present
in `./assembler/`. You can use the executable in the `./assembler/` folder to assemble your program. 

Run `./assembler input_file output_file` in the `./assembler/` folder, replacing `input_file` with the name of your
program and `output_file` with the name of your output file. Optionally, you can add a third option to the program, 
the code offset. However, this is not recommended if you're simply making a user program, as you run the risk of
overwriting core CPU data or executing code you don't mean to. 

Examples:

`./assembler input_file output_file` will assemble `input_file` into `output_file` with a default code offset at 
0xE000, where the CPU will expect the program to be loaded at.

`./assembler input_file output_file 0x0` will assemble `input_file` into `output_file` with a code offset at 0x0. 
This is NOT recommended, as you run the risk of jumping to 0x0, where the start up code is located, and can cause
weird errors of executing code you don't mean to.

The option to use a different code offset was made so I could write assembly for the core components of the CPU.
(Yes! The core instructions that the CPU executes (start up code, "standard library") are written in the language and
loaded into memory directly instead of being written in C!)

Only use the code offset feature if you know what you are doing and are sure of it!!

## Assembling for CPU

## MOV

## STR

## CMP

## JUMPING

## ARITHMETIC

## STACK

## BITWISE

## OTHER