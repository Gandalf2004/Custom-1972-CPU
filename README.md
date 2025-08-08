# Install
## Windows
You need to install the [MinGw Compiler](https://www.mingw-w64.org/) which is GCC for Windows) and [Make for Windows](https://gnuwin32.sourceforge.net/packages/make.htm).

## Arch Linux
Install GCC and Make using pacman:
`sudo pacman -S gcc make`

## Other Linux Distributions:
Please check which package manager your distribution uses and install `gcc` and `make` accordingly.




If you have `make` and `gcc` you should open the console, go to the directory where you have all the files installed and run `make`  then the C source files sould be compieling

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
