#pragma once

#include "types.h"

void* memset(void* addr, u8 val, u32 size);
void* mem_zero(void* addr, u32 size);
void* memcpy(void* dest, const void* src, u32 size);
s32 memcmp(const u8* a, const u8* b, u32 size);
s32 strcmp(const u8* a, const u8* b);
u8* strcpy(u8* dest, const u8* src);
u32 strlen(const u8* str);
void kprintf(const u8* format, ...);

