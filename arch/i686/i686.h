#pragma once

#include <stdint.h>

typedef uint16_t port_t;

#define NULL_PORT ((port_t)(0xFFFF))

static inline void i686_outb(port_t port, uint8_t value) {
    __asm__ volatile ("outb %1, %0" :: "Nd"(port), "a"(value));
}

static inline void i686_outw(port_t port, uint16_t value) {
    __asm__ volatile ("outw %1, %0" :: "Nd"(port), "a"(value));
}

static inline void i686_outl(port_t port, uint32_t value) {
    __asm__ volatile ("outl %1, %0" :: "Nd"(port), "a"(value));
}

static inline uint8_t i686_inb(port_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline uint16_t i686_inw(port_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline uint32_t i686_inl(port_t port) {
    uint32_t ret;
    __asm__ volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Enable interrupts
static inline void i686_sti() {
    __asm__ volatile ("sti" ::: "memory");
}

// Disable interrupts
static inline void i686_cli() {
    __asm__ volatile ("cli" ::: "memory");
}

static inline void i686_iowait() {
    i686_outb(0x80, 0);
}

__attribute__((noreturn)) void i686_panic(void);