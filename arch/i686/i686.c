#include "i686.h"

__attribute__((naked, noreturn))
void i686_panic(void) {
    __asm__ volatile (
        "cli\n"
        "1:\n"
        "hlt\n"
        "jmp 1b\n"
    );
}