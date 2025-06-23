// berryasm.c - simple BEFB assembler
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define BEFB_MAGIC 0x42454642

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint64_t entry_point;
    uint64_t text_offset;
    uint64_t text_size;
    uint64_t data_offset;
    uint64_t data_size;
    uint64_t rodata_offset;
    uint64_t rodata_size;
    uint64_t bss_size;
} __attribute__((packed)) BEFBHeader;

int main(int argc, char *argv[]) {
    if (argc != 4 || strcmp(argv[2], "-o") != 0) {
        printf("Usage: %s <source.s> -o <output.befb>\n", argv[0]);
        return 1;
    }

    const char *input = argv[1];
    const char *output = argv[3];

    FILE *fin = fopen(input, "r");
    if (!fin) {
        perror("fopen");
        return 1;
    }

    FILE *fout = fopen(output, "wb");
    if (!fout) {
        perror("fopen");
        fclose(fin);
        return 1;
    }

    // Prepare .text buffer (64 KB max)
    uint8_t textbuf[65536];
    uint32_t textsize = 0;

    // Prepare .rodata buffer (64 KB max)
    uint8_t rod_buf[65536];
    uint32_t rod_size = 0;

    // Hardcoded example: assemble fixed Hello World program
    // You may replace this with full parser later
    // Instruksi:
    // movz x0, #1        => D2800020
    // movz x2, #13       => D2801A42
    // movz x8, #64       => D2808008
    // svc 0              => D4000001
    // movz x0, #0        => D2800000
    // movz x8, #93       => D280BA08
    // svc 0              => D4000001

    uint32_t code[] = {
        0xD2800020, // movz x0, #1
        0xD2800001, // movz x1, #0 (offset to string)
        0xD2801A42, // movz x2, #13
        0xD2808008, // movz x8, #64
        0xD4000001, // svc 0
        0xD2800000, // movz x0, #0
        0xD280BA08, // movz x8, #93
        0xD4000001  // svc 0
    };

    memcpy(textbuf, code, sizeof(code));
    textsize = sizeof(code);

    // Prepare rodata
    const char *msg = "Hello world!\n";
    memcpy(rod_buf, msg, strlen(msg));
    rod_size = strlen(msg);

    // Header
    BEFBHeader hdr = {
        .magic = BEFB_MAGIC,
        .version = 1,
        .entry_point = 0,
        .text_offset = sizeof(BEFBHeader),
        .text_size = textsize,
        .data_offset = 0,
        .data_size = 0,
        .rodata_offset = sizeof(BEFBHeader) + textsize,
        .rodata_size = rod_size,
        .bss_size = 0
    };

    // Tulis header
    fwrite(&hdr, sizeof(hdr), 1, fout);
    // Tulis .text
    fwrite(textbuf, 1, textsize, fout);
    // Tulis .rodata
    fwrite(rod_buf, 1, rod_size, fout);

    fclose(fin);
    fclose(fout);

    printf("Assembled to: %s (%d bytes .text, %d bytes .rodata)\n", output, textsize, rod_size);
    return 0;
}
