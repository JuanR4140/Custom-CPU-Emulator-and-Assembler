EMU_FILES = main.c emu/cpu.c emu/mem.c
ASM_FILES = assembler/assembler.c
OUT_FILES = ./emu/core/vram.out ./emu/core/jmp_to_code.out

make:
	gcc -g ${EMU_FILES} -o emulator -lcurses
	gcc -g ${ASM_FILES} -o assembler/assembler

	./assembler/assembler ./emu/core/vram.cpu ./emu/core/vram.out 0x0
	./assembler/assembler ./emu/core/jmp_to_code.cpu ./emu/core/jmp_to_code.out
	
clean:
	rm emulator
	rm assembler/assembler
	rm ${OUT_FILES}