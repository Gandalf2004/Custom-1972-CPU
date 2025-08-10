//Copyright © Martin H. Sharp; August 2025
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

bool DEBUG = false;

// ANSI escape codes for colors
    #define RED     "\x1b[31m"
    #define GREEN   "\x1b[32m"
    #define YELLOW  "\x1b[33m"
    #define BLUE    "\x1b[34m"
    #define MAGENTA "\x1b[35m"
    #define CYAN    "\x1b[36m"
    #define RESET   "\x1b[0m"

// Memory: 256 addresses, each 16-bit (instruction)
#define MEM_SIZE 256
uint16_t memory[MEM_SIZE];

// Registers
uint8_t RA = 0, RB = 0, RC = 0, RE = 0;
uint16_t IR  = 0; // Instruction Register
uint8_t PC   = 0; // Program Counter
uint16_t MAR = 0; // Memory Address Register
uint8_t STOFR = 0;// STack OverFlow Register
uint8_t STUFR = (MEM_SIZE -1);// STack UnderFlow Register
uint8_t SP;


// Flags
bool ZF = false; // Zero Flag
bool NF = false; // Negative Flag
bool OF = false; // Overflow Flag

// Control Signals
bool sub_enable = false;

// Bus (for simulation purposes only)
uint8_t bus = 0;

// Opcodes
enum OPCODE {
    MOVR = 0x01,
    MOVA, MOVB, MOVC, MOVE,
    STORA, STORB, STORC, STORE,
    LDIMA, LDIMB, LDIMC, LDIME,
    JMPN, JMPZ, JMPO, JMP,
    ADD, SUB, ADDR, SUBR,
    OUT, CALL, RET, MOVA_PTRB,
    STORA_PTRB, PUSH, POP, 
    ADDSP, SUBSP, SSTOF, SSTUF, HLT = 0xFF
};

// Helpers
void update_flags(int result) {
    ZF = (result == 0);
    NF = (result < 0);
    OF = (result > 255 || result < 0);
}

void alu_add(uint8_t value) {
    int result = RA + value;
    update_flags(result);
    RA = (uint8_t)(result & 0xFF);
}

void alu_sub(uint8_t value) {
    int result = RA - value;
    update_flags(result);
    RA = (uint8_t)(result & 0xFF);
}

uint8_t* get_register(uint8_t code) {
    switch(code) {
        case 0x01: return &RA;
        case 0x02: return &RB;
        case 0x03: return &RC;
        case 0x04: return &RE;
        case 0x05: return &SP;
        default: return NULL;
    }
}

void load_program(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        exit(1);
    }
    size_t loaded = fread(memory, sizeof(uint16_t), MEM_SIZE, file);
    fclose(file);

    if (DEBUG) {
        printf("%sLoaded %zu instructions into memory%s\n", CYAN, loaded, RESET);
    }
}

void execute_instruction() {
    IR = memory[PC++];
    uint8_t opcode = (IR >> 8) & 0xFF;
    uint8_t operand = IR & 0xFF;

    if (DEBUG){
        printf("%sPC=%02X IR=%04X STOFR=%d STUFR=%d %sOPCODE=%02X OPERAND=%02X %sRA=%d RB=%d RC=%d RE=%d %sZF=%d NF=%d OF=%d %sSP= %d%s\n",
           MAGENTA, PC-1, IR, STOFR, STUFR, GREEN, opcode, operand, CYAN, RA, RB, RC, RE, YELLOW, ZF, NF, OF, BLUE, SP, RESET);
    }

    switch(opcode) {

        case MOVR: {
            uint8_t dest = (operand >> 4) & 0x0F;
            uint8_t src = operand & 0x0F;
            uint8_t* d = get_register(dest);
            uint8_t* s = get_register(src);
            if (d && s) *d = *s;
            break;
        }
        case MOVA: RA = memory[operand] & 0xFF; break;
        case MOVB: RB = memory[operand] & 0xFF; break;
        case MOVC: RC = memory[operand] & 0xFF; break;
        case MOVE: RE = memory[operand] & 0xFF; break;
        case STORA: memory[operand] = RA; break;
        case STORB: memory[operand] = RB; break;
        case STORC: memory[operand] = RC; break;
        case STORE: memory[operand] = RE; break;
        case LDIMA: RA = operand; break;
        case LDIMB: RB = operand; break;
        case LDIMC: RC = operand; break;
        case LDIME: RE = operand; break;
        case JMPN: if (NF) PC = operand; break;
        case JMPZ: if (ZF) PC = operand; break;
        case JMPO: if (OF) PC = operand; break;
        case JMP: PC = operand; break;
        case ADD: alu_add(operand); break;
        case SUB: alu_sub(operand); break;
        case ADDR: {
            uint8_t *reg = get_register(operand);
            alu_add(*reg);
            break;
        }
        case SUBR: {
            uint8_t *reg = get_register(operand);
            alu_sub(*reg);
            break;
        }
        case CALL: {
            if ( (SP == STOFR) || (SP == 0) ) {
                printf("%sStack overflow%s\n" , RED, RESET);
                exit(1);
            }
            uint16_t v = (uint16_t)(PC & (MEM_SIZE-1));        // only store low byte into stack cell
            memory[--SP] = v;
            if (DEBUG) printf("  [CALL] push return=0x%02X at mem[%s%u%s]\n", (uint8_t)v, BLUE, SP, RESET);
            PC = operand;
            break;
        }
        case RET: {
            if (SP == MEM_SIZE-1) { printf("%sStack underflow%s\n", RED, RESET); exit(1); }
            uint16_t v = memory[SP++];
            PC = (uint8_t)(v & (MEM_SIZE-1));
            if (DEBUG) printf("  [RET] popped return=0x%02X from mem[%s%u%s]\n", (uint8_t)(v & 0xFF), BLUE, SP-1, RESET);
            break;
        }
        case MOVA_PTRB: {
            RA = (uint8_t)(memory[RB] & (MEM_SIZE-1)); // load low byte of memory[RB];//anding with 0xff
            break;
        }
        case STORA_PTRB: {
            memory[RB] = RA;
            break;
        }
        case PUSH: {
            uint8_t *reg = get_register(operand);
            if ( (SP == STOFR) || (SP == 0) ) { 
                printf("%sStack overflow%s\n", RED, RESET);
                exit(1);
            }
            uint16_t v = (uint16_t)(*reg & (MEM_SIZE-1));
            memory[--SP] = v;
            if (DEBUG) printf("  [PUSH] push 0x%02X into mem[%s%u%s]\n", (uint8_t)v, BLUE, SP, RESET);
            break;
        }
        case POP: {
            uint8_t *reg = get_register(operand);
            if ( (SP == STUFR ) || (SP == (MEM_SIZE -1) ) ) {     
                printf("%sStack underflow%s\n", RED, RESET);
                exit(1);
            }
             uint16_t v = memory[SP++];
            *reg = (uint8_t)(v & (MEM_SIZE-1));
            if (DEBUG) printf("  [POP] pop 0x%02X from mem[%s%u%s]\n", (uint8_t)(v & 0xFF), BLUE, SP-1, RESET);
            break;
        }
        case ADDSP:{
            if ( (SP == STUFR ) || (SP == (MEM_SIZE -1) ) ) {     
                printf("%sStack underflow%s\n", RED, RESET);
                exit(1);
            }
            SP += operand;
            break;
        }
        case SUBSP:{
            if ( (SP == STOFR) || (SP == 0) ) { 
                printf("%sStack overflow%s\n", RED, RESET);
                exit(1);
            }
            SP -= operand;
            break;
        }
        case OUT: printf("%sOUT: %d%s\n", GREEN, RA, RESET); break;
        case SSTOF:{// Set STack OverFlow
            if(operand > 255 || operand < 0) {
                printf("%sIllegal operand for SSTOF(0x%02X): 0x%02X%s\n", RED, opcode, operand, RESET);
                exit(1);
            }
            STOFR = operand;
            break;
        };
        case SSTUF:{// Set STack UnderFlow
            if(operand > 255 || operand < 0) {
                printf("%sIllegal operand for SSTUF(0x%02X): 0x%02X%s\n", RED, opcode, operand, RESET);
                exit(1);
            }
            STUFR = operand;
            SP = operand;
            break;
        };
        case HLT: printf("%sProgramm Halted Execution%s\n", YELLOW, RESET); exit(0);
        default: printf("%sUnknown opcode: 0x%02X%s\n", RED, opcode, RESET); exit(1);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <program.bin> [-d]\n", argv[0]);
        return 1;
    }

    if (argc >= 3 && strcmp(argv[2], "-d") == 0) {
        DEBUG = true;
        printf("%sDEBUG%s mode enabled\n", MAGENTA, RESET);
    }

    SP = STUFR;

    load_program(argv[1]);

    while (1) {
        execute_instruction();
    }

    return 0;
}
