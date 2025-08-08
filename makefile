# Detect OS and distro
ifdef OS
  ifeq ($(OS),Windows_NT)
    OS_NAME := Windows
    EXE_EXT := .exe
    RM := del
  else
    OS_NAME := Unknown
    EXE_EXT :=
    RM := rm -f
  endif
else
  UNAME_S := $(shell uname -s 2>/dev/null)
  ifeq ($(UNAME_S),Linux)
    DISTRO := $(shell grep '^ID=' /etc/os-release 2>/dev/null | cut -d= -f2)
    ifeq ($(DISTRO),arch)
      OS_NAME := ArchLinux
    else
      OS_NAME := Linux
    endif
    EXE_EXT :=
    RM := rm -f
  else
    OS_NAME := Unknown
    EXE_EXT :=
    RM := rm -f
  endif
endif

CC := gcc
CFLAGS := -Os

# Source files and output binaries
ASM_SRC := ASEMBLER.c
ASM_EXE := EC72ASM$(EXE_EXT)

CPU_SRC := Custom_8bit_CPU_EMULATOR.c
CPU_EXE := EC72CPU$(EXE_EXT)

HXDMP_SRC := hexdump.c
HXDMP_EXE := dump$(EXE_EXT)



ifeq ($(OS_NAME),Windows)
all: $(ASM_EXE) $(CPU_EXE) $(HXDMP_EXE)
	@echo "Built on $(OS_NAME)"
else
all: $(ASM_EXE) $(CPU_EXE)
	@echo "Built on $(OS_NAME)"
endif	


$(HXDMP_EXE): $(HXDMP_SRC)
	$(CC) $(CFLAGS) $< -o $@

$(ASM_EXE): $(ASM_SRC)
	$(CC) $(CFLAGS) $< -o $@

$(CPU_EXE): $(CPU_SRC)
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) $(ASM_EXE) $(CPU_EXE) $(HXDMP_EXE)

.PHONY: all clean
