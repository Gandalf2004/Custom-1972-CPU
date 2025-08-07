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
uint16_t IR = 0;  // Instruction Register
uint8_t PC = 0;   // Program Counter
uint16_t MAR = 0; // Memory Address Register
uint8_t SP = (MEM_SIZE -1);

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
    MOVR = 0x01, MOVA, MOVB, MOVC,
    STORA, STORB, STORC,
    LDIMA, LDIMB, LDIMC,
    JMPN, JMPZ, JMPO, JMP,
    ADD, SUB, ADDR, SUBR,
    OUT, CALL, RET, MOVA_PTRB,
    STORA_PTRB, PUSH, POP, 
    ADDSP, SUBSP, HLT = 0xFF
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
        printf("PC=%02X IR=%04X OPCODE=%02X OPERAND=%02X RA=%d RB=%d RC=%d RE=%d ZF=%d NF=%d OF=%d SP= %d\n",
           PC-1, IR, opcode, operand, RA, RB, RC, RE, ZF, NF, OF, SP);
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
        case STORA: memory[operand] = RA; break;
        case STORB: memory[operand] = RB; break;
        case STORC: memory[operand] = RC; break;
        case LDIMA: RA = operand; break;
        case LDIMB: RB = operand; break;
        case LDIMC: RC = operand; break;
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
            if (SP == 0) { printf("%sStack overflow%s\n" , RED, RESET); exit(1); }
            //SP--;
            memory[--SP] = PC;// pre-increment SP after fetching
            PC = operand;
            break;
        }
        case RET: {
            if (SP == MEM_SIZE-1) { printf("%sStack underflow%s\n", RED, RESET); exit(1); }
            PC = memory[SP++]; // post-increment SP after fetching
            break;
        }
        case MOVA_PTRB: {
            RA = memory[RB] & (MEM_SIZE-1);//anding with 0xff
            break;
        }
        case STORA_PTRB: {
            memory[RB] = RA;
            break;
        }
        case PUSH: {
            uint8_t *reg = get_register(operand);
            if (SP == 0) { 
                printf("%sStack overflow%s\n", RED, RESET);
                exit(1);
            }
            //SP--;                     // move SP down first
            memory[--SP] = *reg;        // then store
            break;
        }
        case POP: {
            uint8_t *reg = get_register(operand);
            if (SP == MEM_SIZE-1) {     
                printf("%sStack underflow%s\n", RED, RESET);
                exit(1);
            }
            *reg = memory[SP++];        // read top
            //SP++;                     // then move SP up
            break;
        }
        case ADDSP:{
            if (SP == MEM_SIZE-1) {     
                printf("%sStack underflow%s\n", RED, RESET);
                exit(1);
            }
            SP += operand;
            break;
        }
        case SUBSP:{
            if (SP == 0) { 
                printf("%sStack overflow%s\n", RED, RESET);
                exit(1);
            }
            SP -= operand;
            break;
        }
        case OUT: printf("%sOUT: %d%s\n", GREEN, RA, RESET); break;
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


    load_program(argv[1]);

    while (1) {
        execute_instruction();
    }

    return 0;
}
