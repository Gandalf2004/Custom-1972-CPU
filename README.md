# Compile
On Windoes you need the [MinGw Compiler](https://www.mingw-w64.org/) which is Gcc on Windows, and you need [Make for Windows](https://gnuwin32.sourceforge.net/packages/make.htm).

For Linux you need `gcc`, you can install it with `sudo pacman -S gcc`.

And `make`, you can install it with `sudo pacman -S make`.

## Compile the CPU emulator:
`gcc Custom_8bit_CPU_EMULATOR.c -o CPU.exe -Os`

## Compile the assembler:
`gcc ASSEMBLER.c -o ASM.exe -Os`

## Doku Of the Instruction Set Archetecture (ISA)
the Dokumentation for the ISA is in the [custom_ISA_DOKU.txt](custom_ISA_DOKU.txt) file

# on Windows
## Use the assembler to translate assembly into a binary:
`./ASM.exe ./testprogramm.asm test.bin -d  # with debug`
`./ASM.exe ./testprogramm.asm test.bin    # without debug`

## Run the emulator:
`./CPU.exe test.bin -d  # with debug`
`./CPU.exe test.bin     # without debug`


## On linux
