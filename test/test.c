#include "../kernel/types.h"

typedef void (*kprintf_t)(const u8*, ...);
#define kprintf ((kprintf_t)0x00101D44)

static const u8* msg = "test";

int main(void) {
    kprintf("%s\n", msg);
    return 0;
}
