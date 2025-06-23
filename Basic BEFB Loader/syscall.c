// syscall.c
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include "befb_loader.h"

// Daftar syscall (mengacu ke Linux AArch64 nomor standar atau custom)
#define SYS_WRITE 64
#define SYS_EXIT  93
#define SYS_PUTS  1000  // Custom syscall ID

void emulate_syscall(VirtualCPU *cpu) {
    uint64_t syscall_num = cpu->regs[8]; // x8: syscall ID (Linux ABI)

    switch (syscall_num) {
        case SYS_WRITE: {
            int fd        = cpu->regs[0]; // x0
            uint64_t addr = cpu->regs[1]; // x1
            int len       = cpu->regs[2]; // x2

            if (addr + len > cpu->memory_size) {
                fprintf(stderr, "[ERROR] write: out-of-bounds memory access\n");
                return;
            }

            const char *buf = (const char *)(cpu->memory + addr);
            write(fd, buf, len);
            break;
        }

        case SYS_EXIT: {
            int code = cpu->regs[0]; // x0
            printf("[INFO] Program exited with code %d\n", code);
            exit(code);
            break;
        }

        case SYS_PUTS: {
            uint64_t addr = cpu->regs[0]; // x0
            if (addr >= cpu->memory_size) {
                fprintf(stderr, "[ERROR] puts: invalid memory address\n");
                return;
            }

            const char *str = (const char *)(cpu->memory + addr);
            printf("%s", str);
            break;
        }

        default:
            fprintf(stderr, "[WARN] Unknown syscall: %llu\n", syscall_num);
            break;
    }
}
~/befb/loader $
