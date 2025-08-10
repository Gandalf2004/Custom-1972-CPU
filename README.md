# Install
## Windows
You need to install the [MinGw Compiler](https://www.mingw-w64.org/) which is GCC for Windows) and [Make for Windows](https://gnuwin32.sourceforge.net/packages/make.htm).

## Arch Linux
Install GCC and Make using pacman:
```
sudo pacman -S gcc make
```

## Other Linux Distributions:
Please check which package manager your distribution uses and install `gcc` and `make` accordingly.


------------------------------


If you have both `make` and `gcc` installed, open your terminal, navigate to the directory where you want the source files of this project to be, and run:
```
git clone https://github.com/Gandalf2004/Custom-1972-CPU.git
```
Or download the source files directly via the GitHub-GUI


If you have the source files installed, open your terminal, navigate to the directory where the source files of this project are located, and run:
```
make
```

This will compile the C source files.

------------------------------------------

If you want to recompile the project just run:
```
make clean
```
to delete the executables and then:
```
make
```
to recompile them.


## Documentation of the Instruction Set Architecture (ISA)
The documentation for the ISA is in the [custom_ISA_DOKU.txt](custom_ISA_DOKU.txt) file.

# Usage

## On Windows
### Use the assembler to translate assembly into a binary:
```
.\EC72ASM.exe .\testprogramm.ec72asm test.bin -d  # with debug
.\EC72ASM.exe .\testprogramm.ec72asm test.bin     # without debug
```
### Run the emulator:
```
.\EC72CPU.exe .\test.bin -d  # with debug
.\EC72CPU.exe .\test.bin     # without debug
```

## On Linux
### Use the assembler to translate assembly into a binary:
```
./EC72ASM ./testprogramm.ec72asm test.bin -d  # with debug
./EC72ASM ./testprogramm.ec72asm test.bin     # without debug
```
### Run the emulator:
```
./EC72CPU test.bin -d  # with debug
./EC72CPU test.bin     # without debug
```
## syntax highlighting for the Custom Assembly
look at my other project: [Syntax-highlighter-for-EC72ASM](https://github.com/Gandalf2004/Syntax-highlighter-for-EC72ASM)
