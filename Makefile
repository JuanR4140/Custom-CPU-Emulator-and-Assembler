EMU_FILES = main.c emu/cpu.c emu/mem.c
ASM_FILES = assembler/assembler.c

BOOT_FILES = ./emu/core/boot/vram.out ./emu/core/boot/jmp_to_code.out
STD_FILES  = ./emu/core/stdlib/std_print_char.out \
	     ./emu/core/stdlib/std_print_str.out

make:
	gcc -g ${EMU_FILES} -o emulator -lcurses
	gcc -g ${ASM_FILES} -o assembler/assembler

	./assembler/assembler ./emu/core/boot/vram.cpu ./emu/core/boot/vram.out 0x0
	./assembler/assembler ./emu/core/boot/jmp_to_code.cpu ./emu/core/boot/jmp_to_code.out

	./assembler/assembler ./emu/core/stdlib/std_print_char.cpu ./emu/core/stdlib/std_print_char.out 0x200
	
	./assembler/assembler ./emu/core/stdlib/std_print_str.cpu ./emu/core/stdlib/std_print_str.out 0x260

clean:
	rm emulator
	rm assembler/assembler
	rm ${BOOT_FILES}
	rm ${STD_FILES}
