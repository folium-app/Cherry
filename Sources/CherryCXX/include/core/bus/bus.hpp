/*
 * moestation is a WIP PlayStation 2 emulator.
 * Copyright (C) 2022-2023  Lady Starbreeze (Michelle-Marie Schiller)
 */

#pragma once

#include "core/ee/vif/vif.hpp"
#include "common/types.hpp"

using VectorInterface = ps2::ee::vif::VectorInterface;

namespace ps2::bus {

void init(const char *biosPath, VectorInterface *vif0, VectorInterface *vif1);
void setPathEELOAD(const char *path);

void saveRAM();

u8   read8(u32 addr);
u16  read16(u32 addr);
u32  read32(u32 addr);
u64  read64(u32 addr);
u128 read128(u32 addr);

u8  readIOP8(u32 addr);
u16 readIOP16(u32 addr);
u32 readIOP32(u32 addr);

u32  readDMAC32(u32 addr);
u128 readDMAC128(u32 addr);

void write8(u32 addr, u8 data);
void write16(u32 addr, u16 data);
void write32(u32 addr, u32 data);
void write64(u32 addr, u64 data);
void write128(u32 addr, const u128 &data);

void writeIOP8(u32 addr, u8 data);
void writeIOP16(u32 addr, u16 data);
void writeIOP32(u32 addr, u32 data);

void writeDMAC32(u32 addr, u32 data);
void writeDMAC128(u32 addr, const u128 &data);

}
