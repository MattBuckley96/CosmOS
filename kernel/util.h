#pragma once

#include "types.h"

static void* memset(void* dest, u8 val, u32 size)
{
    u8* temp = (u8*)dest;
    for (u32 i = 0; i < size; i++)
    {
        temp[i] = val;
    }
    return dest;
}