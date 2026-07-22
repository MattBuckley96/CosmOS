#include "../kernel/types.h"
#include "../kernel/console.h"

typedef void (*kprintf_t)(const u8*, ...);
#define kprintf ((kprintf_t)0x001015FC)

int main() {
    kprintf("%FHello%F from another %FWorld!\n%F",
        VGA_LIGHT_YELLOW, VGA_GRAY, VGA_LIGHT_GREEN, VGA_GRAY); 

    return 0;
}
