#include <stdio.h>
#include <stdint.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s file.bin\n", argv[0]);
        return 1;
    }

    FILE* f = fopen(argv[1], "rb");
    if (!f) {
        perror("Failed to open file");
        return 1;
    }

    uint16_t word;
    int address = 0;
    while (fread(&word, sizeof(uint16_t), 1, f) == 1) {
        printf("0x%04X: 0x%04X\n", address, word);
        address++;
    }

    fclose(f);
    return 0;
}