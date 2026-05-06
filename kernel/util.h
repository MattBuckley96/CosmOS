#pragma once

#include "types.h"

void* memset(void* dest, u8 val, u32 size);
void kprintf(const char* fmt, ...);
void print_regs(void);
int strcmp(const char* a, const char* b);