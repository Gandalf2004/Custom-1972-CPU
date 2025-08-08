# Install
## Windows
You need to install the [MinGw Compiler](https://www.mingw-w64.org/) which is GCC for Windows) and [Make for Windows](https://gnuwin32.sourceforge.net/packages/make.htm).

## Arch Linux
Install GCC and Make using pacman:
`sudo pacman -S gcc make`

## Other Linux Distributions:
Please check which package manager your distribution uses and install `gcc` and `make` accordingly.


------------------------------

If you have both `make` and `gcc` installed, open your terminal, navigate to the directory where your files are located, and run:

`make`

This will compile the C source files.

## Doku Of the Instruction Set Archetecture (ISA)
the Dokumentation for the ISA is in the [custom_ISA_DOKU.txt](custom_ISA_DOKU.txt) file

# On Windows
## Use the assembler to translate assembly into a binary:
`./ASM.exe ./testprogramm.asm test.bin -d  # with debug`
`./ASM.exe ./testprogramm.asm test.bin    # without debug`

## Run the emulator:
`./CPU.exe test.bin -d  # with debug`
`./CPU.exe test.bin     # without debug`


## On linux
