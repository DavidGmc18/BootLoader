#include "io.h"

void i686_outb(port_t port, uint8_t value) {
    __asm__ volatile ("outb %1, %0" :: "Nd"(port), "a"(value));
}

void i686_outw(port_t port, uint16_t value) {
    __asm__ volatile ("outw %1, %0" :: "Nd"(port), "a"(value));
}

void i686_outl(port_t port, uint32_t value) {
    __asm__ volatile ("outl %1, %0" :: "Nd"(port), "a"(value));
}

uint8_t i686_inb(port_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

uint16_t i686_inw(port_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

uint32_t i686_inl(port_t port) {
    uint32_t ret;
    __asm__ volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void i686_sti() {
    __asm__ volatile ("sti" ::: "memory");
}

void i686_cli() {
    __asm__ volatile ("cli" ::: "memory");
}

void i686_iowait() {
    i686_outb(0x80, 0);
}

__attribute__((naked, noreturn))
void i686_panic(void) {
    __asm__ volatile (
        "cli\n"
        "1:\n"
        "hlt\n"
        "jmp 1b\n"
    );
}