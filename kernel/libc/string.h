#ifndef STRING_H
#define STRING_H

#include "types.h"

int strcmp(const char* a, const char* b);
void* memmove(void* dest, const void* src, size_t n);
void* memcpy(void* restrict dest, const void* restrict src, size_t n);

#endif
