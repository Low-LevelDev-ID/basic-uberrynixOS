// befb_loader.c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "befb_loader.h"

#define BEFB_MAGIC 0x42454642  // "BEFB"

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

int load_befb(const char *filename, VirtualCPU *cpu) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("Error opening BEFB file");
        return -1;
    }

    // Baca header
    BEFBHeader hdr;
    if (fread(&hdr, sizeof(BEFBHeader), 1, fp) != 1) {
        fprintf(stderr, "Failed to read BEFB header\n");
        fclose(fp);
        return -1;
    }

    // Verifikasi magic
    if (hdr.magic != BEFB_MAGIC) {
        fprintf(stderr, "Invalid BEFB magic: 0x%X\n", hdr.magic);
        fclose(fp);
        return -1;
    }

    // Hitung total ukuran memory virtual
    uint64_t total_size = hdr.text_size + hdr.data_size + hdr.rodata_size + hdr.bss_size + 0x10000; // + extra stack

    cpu->memory = (uint8_t *)calloc(1, total_size);
    if (!cpu->memory) {
        fprintf(stderr, "Failed to allocate virtual memory\n");
        fclose(fp);
        return -1;
    }
    cpu->memory_size = total_size;

    // Load .text
    if (hdr.text_size > 0) {
        fseek(fp, hdr.text_offset, SEEK_SET);
        if (fread(cpu->memory, 1, hdr.text_size, fp) != hdr.text_size) {
            fprintf(stderr, "Failed to load .text section\n");
            fclose(fp);
            return -1;
        }
    }

    // Load .data
    if (hdr.data_size > 0) {
        fseek(fp, hdr.data_offset, SEEK_SET);
        if (fread(cpu->memory + hdr.text_size, 1, hdr.data_size, fp) != hdr.data_size) {
            fprintf(stderr, "Failed to load .data section\n");
            fclose(fp);
            return -1;
        }
    }

    // Load .rodata
    if (hdr.rodata_size > 0) {
        fseek(fp, hdr.rodata_offset, SEEK_SET);
        if (fread(cpu->memory + hdr.text_size + hdr.data_size, 1, hdr.rodata_size, fp) != hdr.rodata_size) {
            fprintf(stderr, "Failed to load .rodata section\n");
            fclose(fp);
            return -1;
        }
    }

    // Set PC ke entry point relatif terhadap base virtual memory
    cpu->pc = hdr.entry_point;

    // Set SP ke akhir memori (stack tumbuh ke bawah)
    cpu->sp = cpu->memory_size - 8;

    fclose(fp);
    return 0;
}
