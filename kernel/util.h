#pragma once

#include "types.h"

void* memset(void* addr, u8 val, u32 size);
void* mem_zero(void* addr, u32 size);
s32 strcmp(const u8* a, const u8* b);
void kprintf(const u8* format, ...);
