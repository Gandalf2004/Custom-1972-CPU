#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

// Detect native endianness at runtime
bool is_little_endian() {
    uint16_t x = 1;
    return *((uint8_t*)&x) == 1;
}

// Swap bytes in a 16-bit word
uint16_t swap_bytes(uint16_t val) {
    return (val >> 8) | (val << 8);
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s file.bin [-le|-be]\n", argv[0]);
        return 1;
    }

    bool native_is_le = is_little_endian();
    bool use_little_endian = native_is_le; // default native
    const char* endian_str;

    if (argc == 3) {
        if (strcmp(argv[2], "-le") == 0) {
            use_little_endian = true;
            endian_str = "Little Endian (forced)";
        } else if (strcmp(argv[2], "-be") == 0) {
            use_little_endian = false;
            endian_str = "Big Endian (forced)";
        } else {
            fprintf(stderr, "Unknown flag: %s\n", argv[2]);
            fprintf(stderr, "Usage: %s file.bin [-le|-be]\n", argv[0]);
            return 1;
        }
    } else {
        endian_str = native_is_le ? "Little Endian (native detected)" : "Big Endian (native detected)";
    }

    printf("Native CPU endianness detected: %s\n", native_is_le ? "Little Endian" : "Big Endian");
    printf("Using dump mode: %s\n\n", endian_str);

    FILE* f = fopen(argv[1], "rb");
    if (!f) {
        perror("Failed to open file");
        return 1;
    }

    uint16_t word;
    int address = 0;
    while (fread(&word, sizeof(uint16_t), 1, f) == 1) {
        if (use_little_endian != native_is_le) {
            // Swap bytes if forced endian differs from native
            word = swap_bytes(word);
        }
        printf("0x%04X: 0x%04X\n", address, word);
        address++;
    }

    fclose(f);
    return 0;
}
