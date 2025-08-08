//Copyright © Martin H. Sharp; August 2025
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define RED        "\x1b[31m"
#define GREEN      "\x1b[32m"
#define YELLOW     "\x1b[33m"
#define BLUE       "\x1b[34m"
#define MAGENTA    "\x1b[35m"
#define CYAN       "\x1b[36m"
#define RESETCOLOR "\x1b[0m"

#define MAX_LINES      1024
#define MAX_LABELS     128
#define MAX_TOKENS     16
#define MAX_TOKEN_LEN  64
#define MAX_LINE_LEN   256

// Debug control (set via -d flag)
typedef int dbg_t;
dbg_t debug_mode = 0;
#define DEBUG_PRINT(fmt, ...) \
    do { if (debug_mode) fprintf(stderr, "%s[DEBUG] %s:%d: %s" fmt "\n", MAGENTA, __FILE__, __LINE__, RESETCOLOR, ##__VA_ARGS__); } while (0)

// Label table
typedef struct {
    char name[MAX_TOKEN_LEN];
    int address;
} Label_t;
static Label_t labels[MAX_LABELS];
static int label_count = 0;

// Register codes
typedef enum {
    REG_NONE = 0, REG_A, REG_B,
    REG_C, REG_E, REG_SP
} Register_t;

static const struct { const char *name; Register_t code; } register_table[] = {
    {"RA", REG_A}, {"RB", REG_B}, {"RC", REG_C}, {"RE", REG_E}, {"SP", REG_SP}
};

// Opcodes
typedef enum {
    OP_MOVR = 0x01,
    OP_MOVA,        OP_MOVB,        OP_MOVC,    OP_MOVE,
    OP_STORA,       OP_STORB,       OP_STORC,   OP_STORE,
    OP_LDIMA,       OP_LDIMB,       OP_LDIMC,   OP_LDIME,
    OP_JMPN,        OP_JMPZ,        OP_JMPO,    OP_JMP,
    OP_ADD,         OP_SUB,         OP_ADDR,    OP_SUBR,
    OP_OUT,         OP_CALL,        OP_RET,     OP_MOVA_PTRB,
    OP_STORA_PTRB,  OP_PUSH,        OP_POP,     OP_ADDSP,
    OP_SUBSP,       OP_HLT = 0xFF
} Opcode_t;

static const struct { const char *name; Opcode_t code; } opcode_table[] = {
    {"MOVR", OP_MOVR},      {"MOVA", OP_MOVA},      {"MOVB", OP_MOVB},
    {"MOVC", OP_MOVC},      {"STORA", OP_STORA},    {"STORB", OP_STORB},
    {"STORC", OP_STORC},    {"LDIMA", OP_LDIMA},    {"LDIMB", OP_LDIMB}, 
    {"LDIMC", OP_LDIMC},    {"JMPN", OP_JMPN},      {"JMPZ", OP_JMPZ},
    {"JMPO", OP_JMPO},      {"JMP", OP_JMP},        {"ADD", OP_ADD}, 
    {"SUB", OP_SUB},        {"ADDR", OP_ADDR},      {"SUBR", OP_SUBR},
    {"OUT", OP_OUT},        {"CALL", OP_CALL},      {"RET", OP_RET},
    {"MOVA_PTRB", OP_MOVA_PTRB},        {"STORA_PTRB", OP_STORA_PTRB},
    {"PUSH", OP_PUSH},      {"POP", OP_POP},        {"ADDSP", OP_ADDSP},
    {"SUBSP", OP_SUBSP},    {"HLT", OP_HLT}
};

// Token container
typedef struct {
    char tok[MAX_TOKENS][MAX_TOKEN_LEN];
    int count;
} TokenLine_t;

// Forward declarations
static void strip_comment(char *s);
static void trim(char *s);
static void tokenize(const char *line, TokenLine_t *tl);
static int find_label(const char *name);
static void add_label(const char *name, int addr);
static Register_t parse_register(const char *r, int line_no);
static Opcode_t parse_opcode(const char *o, int line_no);
static void error(const char *msg, int line_no, const char *token);

#define ENCODE_MOVR(rd, rs) (((rd) << 4) | (rs))

int main(int argc, char *argv[]) {
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: %s input.asm output.bin [-d]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc == 4 && strcmp(argv[3], "-d") == 0) {
        debug_mode = 1;
    }

    FILE *fin = fopen(argv[1], "r");
    if (!fin) {
        perror("Error opening input file");
        return EXIT_FAILURE;
    }

    char raw[MAX_LINES][MAX_LINE_LEN];
    TokenLine_t lines[MAX_LINES];
    int line_count = 0;

    while (line_count < MAX_LINES && fgets(raw[line_count], MAX_LINE_LEN, fin)) {
        strip_comment(raw[line_count]);
        trim(raw[line_count]);
        if (*raw[line_count] != '\0') {
            tokenize(raw[line_count], &lines[line_count]);
            
            DEBUG_PRINT("%sPosition %d tokens count = %d%s", BLUE, line_count, lines[line_count].count, RESETCOLOR);
            for(int t = 0; t < lines[line_count].count; t++){
                DEBUG_PRINT("\t%stok[%d] = %s%s", BLUE, t, lines[line_count].tok[t], RESETCOLOR);
            }

            if (lines[line_count].count > 0) line_count++;
        }
    }
    fclose(fin);
    if (line_count >= MAX_LINES) {
        error("Too many lines", 0, NULL);
    }

    // First pass: label addresses. Each instruction is assumed 1 word (2 bytes).
    int addr = 0;
    for (int i = 0; i < line_count; i++) {
        TokenLine_t *tl = &lines[i];
        if (tl->count == 0) continue;
        char *first = tl->tok[0];
        size_t len = strlen(first);
        if (first[len - 1] == ':') {
            first[len - 1] = '\0';
            add_label(first, addr);
            DEBUG_PRINT("%sLabel '%s' -> Address 0x%02X(%d)%s", YELLOW, first, addr, addr, RESETCOLOR);
            tl->count = 0;
        } else {
            addr++;  // one word(2 Bytes) per instruction
        }
    }

    // Second pass: assemble
    uint16_t out[MAX_LINES];
    int out_count = 0;

    for (int i = 0; i < line_count; i++) {

        TokenLine_t *tl = &lines[i];
        if (tl->count == 0) continue;
        char *mnemonic = tl->tok[0];
        Opcode_t op = parse_opcode(mnemonic, i);
        uint8_t operand = 0;

        switch (op) {
            case OP_PUSH:
            case OP_POP:
            case OP_ADDR:
            case OP_SUBR:
                if (tl->count < 2) error("Missing register operand", i, mnemonic);
                operand = parse_register(tl->tok[1], i);
                DEBUG_PRINT("%s%s(0x%02X) register %u -> operand=0x%02X%s", CYAN, mnemonic, op, operand, operand, RESETCOLOR);
                break;

            case OP_MOVR: {
                if (tl->count < 3) error("MOVR needs two registers", i, NULL);
                Register_t rd = parse_register(tl->tok[1], i);
                Register_t rs = parse_register(tl->tok[2], i);
                operand = ENCODE_MOVR(rd, rs);
                DEBUG_PRINT("%sMOVR(0x%02X) rd=%u, rs=%u -> operand=0x%02X%s", CYAN, op, rd, rs, operand, RESETCOLOR);
                break;
            }

            case OP_MOVA: case OP_MOVB: case OP_MOVC: case OP_MOVE:
            case OP_STORA: case OP_STORB: case OP_STORC: case OP_STORE:
            case OP_LDIMA: case OP_LDIMB: case OP_LDIMC: case OP_LDIME:
            case OP_ADD: case OP_SUB: case OP_JMP:
            case OP_JMPN: case OP_JMPZ: case OP_JMPO:
            case OP_ADDSP: case OP_SUBSP:
            case OP_CALL: {

                if (tl->count < 2) error("Missing operand", i, mnemonic);

                char *arg = tl->tok[1];
                if (isalpha((unsigned char)arg[0])) {

                    int v = find_label(arg);
                    if (v < 0) error("Undefined label", i, arg);
                    operand = (uint8_t)v;

                } else {
                    if (strlen(arg) > 2 && arg[0]=='0' && (arg[1]=='x'||arg[1]=='X'))
                        operand = (uint8_t)strtol(arg, NULL, 16);
                    else
                        operand = (uint8_t)atoi(arg);
                }

                DEBUG_PRINT("%s%s(0x%02X) -> operand=0x%02X%s", CYAN, mnemonic, op, operand, RESETCOLOR);
                break;
            }

            case OP_RET:
            case OP_MOVA_PTRB:
            case OP_STORA_PTRB:
            case OP_OUT:
            case OP_HLT:
                /* Single-word no-operand or default register operation */
                if (op == OP_OUT && tl->count > 1)
                    operand = parse_register(tl->tok[1], i);
                DEBUG_PRINT("%s%s(0x%02X) -> operand=0x%02X%s", CYAN, mnemonic, op, operand, RESETCOLOR);
                break;

            default:
                error("Unsupported opcode", i, mnemonic);
        }
        out[out_count++] = ((uint16_t)op << 8) | operand;
    }

    FILE *fout = fopen(argv[2], "wb");
    if (!fout) {
        perror("Error opening output file");
        return EXIT_FAILURE;
    }
    fwrite(out, sizeof(uint16_t), out_count, fout);
    fclose(fout);

    printf("%sAssembled %d instructions.%s\n", GREEN, out_count, RESETCOLOR);
    return EXIT_SUCCESS;
}

static void strip_comment(char *s) {
    char *p = strchr(s, ';');
    if (p) *p = '\0';
}

static void trim(char *s) {
    char *start = s;
    while (isspace((unsigned char)*start)) start++;
    memmove(s, start, strlen(start) + 1);
    char *end = s + strlen(s) - 1;
    while (end >= s && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
}

static void tokenize(const char *line, TokenLine_t *tl) {
    tl->count = 0;
    char buf[MAX_LINE_LEN];
    strcpy(buf, line);
    char *p = strtok(buf, " \t,");
    while (p && tl->count < MAX_TOKENS) {
        if (*p) strcpy(tl->tok[tl->count++], p);
        p = strtok(NULL, " \t,");
    }
}

static int find_label(const char *name) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].name, name) == 0)
            return labels[i].address;
    }
    return -1;
}

static void add_label(const char *name, int addr) {
    if (label_count >= MAX_LABELS) error("Too many labels", 0, name);
    strncpy(labels[label_count].name, name, MAX_TOKEN_LEN-1);
    labels[label_count].name[MAX_TOKEN_LEN-1] = '\0';
    labels[label_count++].address = addr;
}

static Register_t parse_register(const char *r, int line_no) {
    if      (strcmp(r, "RA") == 0) return REG_A;
    else if (strcmp(r, "RB") == 0) return REG_B;
    else if (strcmp(r, "RC") == 0) return REG_C;
    else if (strcmp(r, "RE") == 0) return REG_E;
    else if (strcmp(r, "SP") == 0) return REG_SP;

    fprintf(stderr, RED "Error: Invalid register '%s' at line %d\n" RESETCOLOR,
            r, line_no);
    exit(EXIT_FAILURE);
}

static Opcode_t parse_opcode(const char *o, int line_no) {
    for (size_t i = 0; i < sizeof(opcode_table)/sizeof(opcode_table[0]); i++) {
        if (strcmp(opcode_table[i].name, o) == 0)
            return opcode_table[i].code;
    }
    error("Unknown opcode", line_no, o);
    return OP_HLT;
}

static void error(const char *msg, int line_no, const char *token) {
    if (token)
        fprintf(stderr, "%sError: %s at Position %d near '%s'%s\n", RED, msg, line_no, token, RESETCOLOR);
    else
        fprintf(stderr, "%sError: %s at line %d%s\n", RED, msg, line_no, RESETCOLOR);
    exit(EXIT_FAILURE);
}
