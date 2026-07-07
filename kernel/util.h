#pragma once

#include "types.h"

void* memset(void* addr, u8 val, u32 size);
void* mem_zero(void* addr, u32 size);
void kprintf(const u8* format, ...);
