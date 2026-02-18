#ifndef STRING_H
#define STRING_H

#include "types.h"

void* memmove(void* dest, const void* src, size_t n);
void* memcpy(void* restrict dest, const void* restrict src, size_t n);
void* memset(void *mem, int c, size_t n);

int strcmp(const char* a, const char* b);
size_t strlen(const char* s);
char* strcpy(char* dest, const char* src);

#endif
