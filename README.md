You would have to compile the emulator yourself with GCC

you can compile the CPU via `gcc Custom_8bit_CPU_EMULATOR.c -o CPU.exe -Os`
you can compile the assembler via `gcc ASSEMBLER.c -o ASM.exe -Os`

then use the assembler with `.\ASM.exe .\testprogramm.asm test.bin -d` on windows with debug option, or without the `-d` flag with out the debug option
then use the EMULATOR with `.\CPU.exe  .\test.bin -d` on windows with debug option, or without the `-d` flag with out the debug option
