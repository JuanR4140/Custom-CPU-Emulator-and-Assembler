EMU_FILES = main.c emu/cpu.c emu/mem.c
ASM_FILES = assembler/assembler.c
OUT_FILES = ./emu/core/vram.out

make:
	gcc ${EMU_FILES} -o emulator
	gcc ${ASM_FILES} -o assembler/assembler

	./assembler/assembler ./emu/core/vram.cpu ./emu/core/vram.out 0x0
	
clean:
	rm emulator
	rm assembler/assembler
	rm ${OUT_FILES}