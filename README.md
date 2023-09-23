# Custom CPU Emulator and Assembler

Written by JuanR4140

This `README.md` is subject to change as I see fit during the development of the project.

## Table of Contents

- [About](#about)
- [Installation](#installation)
- [Usage](#usage)
- [Roadmap](#roadmap)

## About

This project is really two projects in one, a custom CPU emulator and an assembler for the custom CPU.

The custom CPU has its own insruction set, with its own assembly-like language.
The documentation for the language can be found in `DOCS.md`.

However, the language is a bit tedious to work with, as there are no easy support for strings or variables
(having to deal with memory locations directly).

## Installation

Enter the root folder of the project, then run `make`.

`make` will automatically compile all the code, including the emulator, assembler, and even core CPU files!

## Usage

Run `./emulator` in the root folder. The emulator will starting emulating the CPU, which will
execute some start-up code in memory (like setting up VRAM) before jumping to memory location 0xE000, 
where the user program will be loaded at.

## Roadmap

### Emulator

- [X] Implemented CPU structure
- [X] Implemented memory structure
- [X] Implemented instruction set
- [X] Implementing emulating instruction set
- [X] Implementing start-up code
- [X] Implementing stack
- [ ] Implementing "standard lib"

- [ ] Done

### Assembler
- [X] Implemented instruction set
- [X] Implemented assembly to machine code
- [X] Implemented writing machine code to file
- [X] Implemented optional code offsets

- [X] Done