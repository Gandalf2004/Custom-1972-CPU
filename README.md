# Compile
## Compile the CPU emulator:
`gcc Custom_8bit_CPU_EMULATOR.c -o CPU.exe -Os`

## Compile the assembler:
`gcc ASSEMBLER.c -o ASM.exe -Os`

# on Windows
## Use the assembler to translate assembly into a binary:
`./ASM.exe ./testprogramm.asm test.bin -d  # with debug`
`./ASM.exe ./testprogramm.asm test.bin    # without debug`

## Run the emulator:
`./CPU.exe test.bin -d  # with debug`
`./CPU.exe test.bin     # without debug`


## On linux
