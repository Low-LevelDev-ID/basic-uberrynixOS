#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "befb_loader.h"
#include "syscall.h"

// Fungsi bantu untuk ambil 32-bit dari memory
static inline uint32_t fetch_instruction(VirtualCPU *cpu) {
    if (cpu->pc + 4 > cpu->memory_size) {
        fprintf(stderr, "PC out of bounds: 0x%llx\n", cpu->pc);
        return 0xD65F03C0; // RET
    }
    return *(uint32_t*)(cpu->memory + cpu->pc);
}

// Emulasi instruksi satu per satu
int emulate_instruction(VirtualCPU *cpu) {
    uint32_t inst = fetch_instruction(cpu);

    // Advance PC by 4 for next instruction
    cpu->pc += 4;

    // Decode opcode - hanya contoh, bukan decode penuh AArch64
    if ((inst & 0xFFFFFC00) == 0xD65F0000) {
        // RET
        return 1; // end execution
    }

    // MOVZ xN, #imm
    if ((inst & 0xFFC00000) == 0xD2800000) {
        int rd = (inst >> 0) & 0x1F;
        uint16_t imm16 = (inst >> 5) & 0xFFFF;
        cpu->regs[rd] = imm16;
        return 0;
    }

    // ADD xN, xM, #imm
    if ((inst & 0x7F000000) == 0x11000000) {
        int rd = (inst >> 0) & 0x1F;
        int rn = (inst >> 5) & 0x1F;
        uint16_t imm12 = (inst >> 10) & 0xFFF;
        cpu->regs[rd] = cpu->regs[rn] + imm12;
        return 0;
    }

    // SUB xN, xM, #imm
    if ((inst & 0x7F000000) == 0x51000000) {
        int rd = (inst >> 0) & 0x1F;
        int rn = (inst >> 5) & 0x1F;
        uint16_t imm12 = (inst >> 10) & 0xFFF;
        cpu->regs[rd] = cpu->regs[rn] - imm12;
        return 0;
    }

    // STR xN, [SP, #imm]
    if ((inst & 0xFFC00000) == 0xF80003E0) {
        int rt = (inst >> 0) & 0x1F;
        uint16_t imm = ((inst >> 10) & 0xFFF) << 3;
        uint64_t addr = cpu->sp + imm;
        if (addr + 8 <= cpu->memory_size)
            *(uint64_t*)(cpu->memory + addr) = cpu->regs[rt];
        return 0;
    }

    // LDR xN, [SP, #imm]
    if ((inst & 0xFFC00000) == 0xF94003E0) {
        int rt = (inst >> 0) & 0x1F;
        uint16_t imm = ((inst >> 10) & 0xFFF) << 3;
        uint64_t addr = cpu->sp + imm;
        if (addr + 8 <= cpu->memory_size)
            cpu->regs[rt] = *(uint64_t*)(cpu->memory + addr);
        return 0;
    }

    // SVC #imm
    if ((inst & 0xFFFF0000) == 0xD4000000) {
        emulate_syscall(cpu); // akan handle berdasarkan regs[8]
        return 0;
    }

    // Tidak dikenali
    fprintf(stderr, "Unknown instruction: 0x%08X at PC=0x%llx\n", inst, cpu->pc - 4);
    return -1;
}

// Fungsi utama loop VM
void run_cpu(VirtualCPU *cpu) {
    while (1) {
        int status = emulate_instruction(cpu);
        if (status == 1) {
            break; // Selesai
        } else if (status < 0) {
            fprintf(stderr, "Fatal emulator error. Halting.\n");
            break;
        }
    }
}
