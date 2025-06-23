// main.c
#include <stdio.h>
#include <stdlib.h>
#include "befb_loader.h"
#include "emu_cpu.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <file.befb>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];

    // Inisialisasi CPU virtual
    VirtualCPU cpu;
    for (int i = 0; i < 31; i++) cpu.regs[i] = 0;
    cpu.pc = 0;
    cpu.sp = 0;
    cpu.memory = NULL;
    cpu.memory_size = 0;

    // Load file .befb
    if (load_befb(filename, &cpu) != 0) {
        fprintf(stderr, "Failed to load BEFB file.\n");
        return 1;
    }

    printf("[INFO] Loaded BEFB file: %s\n", filename);
    printf("[INFO] Entry point: 0x%llx\n", cpu.pc);
    printf("[INFO] Memory size: %llu bytes\n", cpu.memory_size);

    // Jalankan CPU
    run_cpu(&cpu);

    // Cleanup
    free(cpu.memory);
    return 0;
}
