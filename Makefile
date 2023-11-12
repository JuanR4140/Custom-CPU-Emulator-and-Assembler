EMU_FILES = main.c emu/cpu.c emu/mem.c
ASM_FILES = assembler/assembler.c
OUT_FILES = ./emu/core/boot/vram.out ./emu/core/boot/jmp_to_code.out

make:
	gcc -g ${EMU_FILES} -o emulator -lcurses
	gcc -g ${ASM_FILES} -o assembler/assembler

	./assembler/assembler ./emu/core/boot/vram.cpu ./emu/core/boot/vram.out 0x0
	./assembler/assembler ./emu/core/boot/jmp_to_code.cpu ./emu/core/boot/jmp_to_code.out
	
clean:
	rm emulator
	rm assembler/assembler
	rm ${OUT_FILES}