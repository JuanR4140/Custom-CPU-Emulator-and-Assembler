# Documentation for the Custom CPU

## Table of Contents

- [About](#about)
- [Memory layout](#memory-layout)
- [General assembling](#general-assembling)
- [Assembling for CPU](#assembling-for-cpu)
- [MOV](#mov)
- [STR](#str)
- [CMP](#cmp)
- [JUMPING](#jumping)
- [PROCEDURES](#procedures)
- [ARITHMETIC](#arithmetic)
- [STACK](#stack)
- [BITWISE](#bitwise)
- [OTHER](#other)

## About

This documentation will go over how the memory layout looks like, how assembling a program looks like,
how setting up a program for the CPU looks like, and how the instruction set as a whole for the Custom CPU
looks like, ordered by type of instruction.

## Memory layout

The memory layout is as follows:

```
0000 - 0100 -> Start up code
0100 - 0200 -> Stack
0200 - 0300 -> Library functions
0300 - 03C8 -> VRAM
0400 - E000 -> Free RAM
E000 - FFFF -> Program code
```

Start up code is where the start up code for the CPU resides, like for setting up VRAM.

Stack is where the stack is located in RAM. When you `push`/`pop`, data is written here.

Library functions is where the "standard library" is, like printing a character to the screen, or clearing it.

VRAM is where video memory resides. These 200 bytes get printed to the screen in a 20x10 ASCII display.
Writing to VRAM directly can be used to write to the screen as an alternative to library functions.

Free RAM is general purpose RAM that can be used by the program. If you want to save variables and such,
this area in memory is where you'd write to.

Program code is the area in memory the emulator will load your program into. When assembling a program, keep
the code offset at the default (0xE000), otherwise you could have errors.

## General assembling

There is no easy support for strings or variables, so you'll have to write to memory locations directly!

To assemble a program, first compile the project with `make` on the root folder so the executable is present
in `./assembler/`. You can use the executable in the `./assembler/` folder to assemble your program. 

Run `./assembler input_file output_file` in the `./assembler/` folder, replacing `input_file` with the name of your
program and `output_file` with the name of your output file. Optionally, you can add a third option to the assembler, 
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

Now that you know how to assemble a program using the assembler, you can now use it to assemble programs for the CPU!

The concept is the same, although you'll have to have a specific name for the file and place it in a specific folder.

Let's say you have this program, `hello_world.cpu`, a 'Hello World' type of program.
```
Coming soon...
```

Assemble the program with the assembler, naming the output file `cartridge.out`. The command should look like:
```
./assembler hello_world.cpu cartridge.out
```

Once assembled, move the `cartridge.out` file into the `./emu/slot/` folder. The file will be automatically loaded into memory
at memory location 0xE000! (Haha! Get it? Like you're putting a cartridge into a cartridge slot? Haha! No? Okay..)

## MOV

`mov dst, src`

Let's learn some instructions then! The most basic of instructions is the `mov` or move instruction, which moves a value into a register. There are 3 types of `mov` instructions.

1. `mov ax, 0x5` moves the literal value `0x5` into register `ax`.

1. `mov ax, bx` moves the register `bx` into register `ax`.

1. `mov ax, $0x400` moves the contents of the address `$0x400` into register `ax`.

## STR

`str src, dst`

`str` stores the contents of a register into a memory address or another register. There are 6 types of `str` instructions.

1. `str ax, bx` stores the contents of register `ax` into the contents of the address held by register `bx`.

1. `sth ax, bx` stores the contents of register `ax`'s high 8 bits into the contents of the address held by register `bx`.

1. `stl ax, bx` stores the contents of register `ax`'s low 8 bits into the contents of the address held by reigster `bx`.

1. `str ax, $0x400` stores the contents of register `ax` into the address `$0x400`.

1. `sth, ax, $0x400` stores the contents of register `ax`'s high 8 bits into the address `$0x400`.

1. `stl ax, $0x400` stores the contents of register `ax`'s low 8 bits into the address `$0x400`.

## CMP

`cmp reg, val`

`cmp` compares a value with a register and sets the appropiate flags. It is used in pair with jumping/branching instructions. There are 3 types of `cmp` instructions.

1. `cmp ax, 0x5` compares the register `ax` with the literal `0x5`.

1. `cmp ax, bx` compares the register `ax` with the register `bx`.

1. `cmp ax, $0x400` compares the register `ax` with the address `$0x400`.

## JUMPING

`jmp label` (and similar)

`jmp` (and similar instructions) are used in pair with the `cmp` instruction, deciding whether to jump or not based on certain flags set by `cmp`. There are 5 types of `jmp` instructions.

1. `jmp label` unconditionally jumps to label.

1. `je label` jumps to `label` if the result of the previous `cmp` were equal.

1. `jne label` jumps to `label` if the result of the previous `cmp` were not equal.

1. `jl label` jumps to `label` if the result of the previous `cmp` was less than.

1. `jg label` jumps to `label` if the result of the previous `cmp` was greater than.

## PROCEDURES

`call label`

`ret`

`call` and `ret` are used for function calling and returning. 

1. `call label` saves the current address on the stack and jumps to `label`

1. `ret` pops the previously saved address and jumps to it, resuming execution. If you `push`ed some values onto the stack for procedure variables, you **must** `ret [bytes]` to clean the stack. (Procedures are "callee cleans up").

## ARITHMETIC

`add` (and similar)

While addition and subtraction work as you would expect, multiplication and division work a little differently as they are *implicit* instructions.

1. `add ax, 0x5` adds the literal `0x5` into the register `ax`.

1. `add ax, bx` adds the register `bx` into the register `ax`.

1. `sub ax, 0x5` subtracts the literal `0x5` from the register `ax`.

1. `sub ax, bx` subtracts the register `bx` from the register `ax`.

1. `mul cx` multiplies the register `ax` with the register `cx`. Because the result could be greater than a 16-bit digit, the result is stored as `dx:ax`, where the high 16 bits are stored in the register `dx` and the low 16 bits are stored in the register `ax`.

1. `div cx` divides the register `ax` by the register `cx`. The result is stored in register `dx`, with the remainder being stored in register `ax`.

## STACK

`push val`

`pop reg`

The `push` and `pop` instructions can be used to interact with the stack, where `push` is used to push data onto the stack, and `pop` is used to pop data from the stack into a register. The `push` instruction can also be used to push data onto the stack to be used by a procedure.

1. `push 0x5` pushes the literal `0x5` onto the stack.

1. `push ax` pushes the contents of the register `ax` onto the stack.

1. `pop ax` pops the value at the top of the stack into the register `ax`.

## BITWISE

`and` (and similar)

The bitwise instructions can be used to directly manipulate
bits in registers. The results of the operations are stored
directly on the register.

1. `and ax, bx` AND's the register `bx` with the register `ax`, storing the result in `ax`.

1. `not ax` NOT's the register `ax`.

1. `or ax, bx` OR's the register `bx` with the register `ax`, storing the result in `ax`.

1. `xor ax, bx` XOR's the register `bx` with the register `ax`, storing the result in `ax`.

1. `shr ax, 0x5` shifts the register `ax` `0x5` bits to the right.

1. `shr ax, bx` shifts the register `ax` the contents of the register `bx` to the right.

1. `shl ax, 0x5` shifts the register `ax` `0x5` bits to the left.

1. `shl ax, bx` shifts the register `ax` the contents of the register `bx` to the left.

## OTHER

`hlt`

`nop`

1. `hlt` halts the CPU. A forced reset is require to continue
operating.

2. `nop` does nothing. `nop` stands for no operation.