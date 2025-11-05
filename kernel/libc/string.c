#include "string.h"

int strcmp(const char* a, const char* b)
{
    while (*a && *b && *a == *b)
    {
        a++;
        b++;
    }
    return (int)(*a) - (int)(*b);
}

void* memmove(void* dest, const void* src, size_t n)
{
    char* d = (char*)dest;
    const char* s = (const char*)src;

    if (d == s || n == 0)
    {
        return dest;
    }

    if (d < s)
    {
        for (size_t i = 0; i < n; i++)
        {
            d[i] = s[i];
        }
    } 
    else
    {
        for (size_t i = n; i > 0; i--)
        {
            d[i - 1] = s[i - 1];
        }
    }

    return dest;
}

void* memcpy(void* restrict dest, const void* restrict src, size_t n)
{
    char* d = (char*)dest;
    const char* s = (const char*)src;

    for (size_t i = 0; i < n; i++)
    {
        d[i] = s[i];
    }

    return dest;
}
