#include "../kernel/types.h"
#include "../kernel/console.h"

typedef void (*kprintf_t)(const u8*, ...);
#define kprintf ((kprintf_t)0x00100A5C)

int main() {
    kprintf("%FHello%F from another %FWorld!\n",
        VGA_LIGHT_YELLOW, VGA_GRAY, VGA_LIGHT_GREEN); 

    return 0;
}
