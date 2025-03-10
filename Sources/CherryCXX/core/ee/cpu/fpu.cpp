/*
 * moestation is a WIP PlayStation 2 emulator.
 * Copyright (C) 2022-2023  Lady Starbreeze (Michelle-Marie Schiller)
 */

#include "core/ee/cpu/fpu.hpp"

#include <cassert>
#include <cmath>
#include <cstdio>

namespace ps2::ee::cpu::fpu {

/* --- FPU constants --- */

constexpr auto doDisasm = true;

/* --- FPU instructions --- */

enum FPUOpcode {
    ADD  = 0x00,
    SUB  = 0x01,
    MUL  = 0x02,
    DIV  = 0x03,
    SQRT = 0x04,
    MOV  = 0x06,
    NEG  = 0x07,
    ADDA = 0x18,
    MADD = 0x1C,
    CVTW = 0x24,
    C    = 0x30,
};

enum class Cond {
    F, EQ, LT, LE,
};

const char *condStr[4] = { "F", "EQ", "LT", "LE" };

/* --- FPU registers --- */

u32 fprs[32];
f32 acc;

bool cpcond1;

/// Get Fd field
u32 getFd(u32 instr) {
    return (instr >> 6) & 0x1F;
}

/// Get Fs field
u32 getFs(u32 instr) {
    return (instr >> 11) & 0x1F;
}

/// Get Ft field
u32 getFt(u32 instr) {
    return (instr >> 16) & 0x1F;
}

void setAcc(f32 data) {
    std::printf("[FPU       ] ACC = %f\n", data);

    acc = data;
}

u32 get(u32 idx) {
    return fprs[idx];
}

f32 getF32(u32 idx) {
    return *(f32 *)&fprs[idx];
}

u32 getControl(u32 idx) {
    switch (idx) {
        case 31:
            std::printf("[FPU       ] Control read @ FCR31\n");
            return 0;
        default:
            std::printf("[FPU       ] Unhandled control read @ %u\n", idx);

            exit(0);
    }
}

void set(u32 idx, u32 data) {
    std::printf("[FPU       ] %u = 0x%08X (%f)\n", idx, data, *(f32 *)&data);

    fprs[idx] = data;
}

void setF32(u32 idx, f32 data) {
    std::printf("[FPU       ] %u = %f (0x%08X)\n", idx, data, *(u32 *)&data);

    fprs[idx] = *(u32 *)&data;
}

void setControl(u32 idx, u32 data) {
    switch (idx) {
        case 31:
            std::printf("[FPU       ] Control write @ FCR31 = 0x%08X\n", data);
            break;
        default:
            std::printf("[FPU       ] Unhandled control write @ %u = 0x%08X\n", idx, data);

            exit(0);
    }
}

bool getCPCOND() {
    return cpcond1;
}

/* ADD */
void iADD(u32 instr) {
    const auto fd = getFd(instr);
    const auto fs = getFs(instr);
    const auto ft = getFt(instr);

    if (doDisasm) {
        std::printf("[FPU       ] ADD $%u, $%u, $%u\n", fd, fs, ft);
    }

    setF32(fd, getF32(fs) + getF32(ft));
}

/* ADD Accumulator */
void iADDA(u32 instr) {
    const auto fs = getFs(instr);
    const auto ft = getFt(instr);

    if (doDisasm) {
        std::printf("[FPU       ] ADDA $%u, $%u\n", fs, ft);
    }

    setAcc(getF32(fs) + getF32(ft));
}

/* Compare */
template<Cond cond>
void iC(u32 instr) {
    const auto fs = getFs(instr);
    const auto ft = getFt(instr);

    const auto s = getF32(fs);
    const auto t = getF32(ft);

    if constexpr (cond == Cond::F) {
        cpcond1 = false;
    }
    if constexpr (cond == Cond::EQ) {
        cpcond1 = s == t;
    }
    if constexpr (cond == Cond::LT) {
        cpcond1 = s < t;
    }
    if constexpr (cond == Cond::LE) {
        cpcond1 = s <= t;
    }

    if (doDisasm) {
        std::printf("[FPU       ] C.%s.S $%u, $%u; $%u = %f, $%u = %f\n", condStr[static_cast<int>(cond)], fs, ft, fs, s, ft, t);
    }
}

/* ConVerT to Single */
void iCVTS(u32 instr) {
    const auto fd = getFd(instr);
    const auto fs = getFs(instr);

    if (doDisasm) {
        std::printf("[FPU       ] CVT.S.W $%u, $%u\n", fd, fs);
    }

    setF32(fd, (f32)(i32)get(fs));
}

/* ConVerT to Word */
void iCVTW(u32 instr) {
    const auto fd = getFd(instr);
    const auto fs = getFs(instr);

    if (doDisasm) {
        std::printf("[FPU       ] CVT.W.S $%u, $%u\n", fd, fs);
    }

    set(fd, (u32)(i32)std::rintf(getF32(fs)));
}

/* DIVide */
void iDIV(u32 instr) {
    const auto fd = getFd(instr);
    const auto fs = getFs(instr);
    const auto ft = getFt(instr);

    if (doDisasm) {
        std::printf("[FPU       ] DIV $%u, $%u, $%u\n", fd, fs, ft);
    }

    setF32(fd, getF32(fs) / getF32(ft));
}

/* Multiply-ADD */
void iMADD(u32 instr) {
    const auto fd = getFd(instr);
    const auto fs = getFs(instr);
    const auto ft = getFt(instr);

    if (doDisasm) {
        std::printf("[FPU       ] MADD $%u, $%u, $%u\n", fd, fs, ft);
    }

    setF32(fd, getF32(fs) * getF32(ft) + acc);
}

/* MOVe */
void iMOV(u32 instr) {
    const auto fd = getFd(instr);
    const auto fs = getFs(instr);

    if (doDisasm) {
        std::printf("[FPU       ] MOV $%u, $%u\n", fd, fs);
    }

    setF32(fd, getF32(fs));
}

/* MULtiply */
void iMUL(u32 instr) {
    const auto fd = getFd(instr);
    const auto fs = getFs(instr);
    const auto ft = getFt(instr);

    if (doDisasm) {
        std::printf("[FPU       ] MUL $%u, $%u, $%u\n", fd, fs, ft);
    }

    setF32(fd, getF32(fs) * getF32(ft));
}

/* NEGate */
void iNEG(u32 instr) {
    const auto fd = getFd(instr);
    const auto fs = getFs(instr);

    if (doDisasm) {
        std::printf("[FPU       ] NEG $%u, $%u\n", fd, fs);
    }

    setF32(fd, -getF32(fs));
}

/* SQuare RooT */
void iSQRT(u32 instr) {
    const auto fd = getFd(instr);
    const auto ft = getFt(instr);

    if (doDisasm) {
        std::printf("[FPU       ] SQRT $%u, $%u\n", fd, ft);
    }

    setF32(fd, std::sqrt(getF32(ft)));
}

/* SUBtract */
void iSUB(u32 instr) {
    const auto fd = getFd(instr);
    const auto fs = getFs(instr);
    const auto ft = getFt(instr);

    if (doDisasm) {
        std::printf("[FPU       ] SUB $%u, $%u, $%u\n", fd, fs, ft);
    }

    setF32(fd, getF32(fs) - getF32(ft));
}

void executeSingle(u32 instr) {
    const auto opcode = instr & 0x3F;

    switch (opcode) {
        case FPUOpcode::ADD  : iADD(instr); break;
        case FPUOpcode::SUB  : iSUB(instr); break;
        case FPUOpcode::MUL  : iMUL(instr); break;
        case FPUOpcode::DIV  : iDIV(instr); break;
        case FPUOpcode::SQRT : iSQRT(instr); break;
        case FPUOpcode::MOV  : iMOV(instr); break;
        case FPUOpcode::NEG  : iNEG(instr); break;
        case FPUOpcode::ADDA : iADDA(instr); break;
        case FPUOpcode::MADD : iMADD(instr); break;
        case FPUOpcode::CVTW : iCVTW(instr); break;
        case FPUOpcode::C + 0: iC<Cond::F>(instr); break;
        case FPUOpcode::C + 2: iC<Cond::EQ>(instr); break;
        case FPUOpcode::C + 4: iC<Cond::LT>(instr); break;
        case FPUOpcode::C + 6: iC<Cond::LE>(instr); break;
        default:
            std::printf("[FPU       ] Unhandled Single instruction 0x%02X (0x%08X)\n", opcode, instr);

            exit(0);
    }
}

void executeWord(u32 instr) {
    const auto opcode = instr & 0x3F;

    assert(opcode == 0x20); // Only CVT.S.W?

    iCVTS(instr);
}

}
