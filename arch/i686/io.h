#pragma once

#include <stdint.h>

typedef uint16_t port_t;

#define NULL_PORT ((port_t)(0xFFFF))

// TODO can some of this be macro instead of functions? would that be better?
void i686_outb(port_t port, uint8_t value);
void i686_outw(port_t port, uint16_t value);
void i686_outl(port_t port, uint32_t value);

uint8_t i686_inb(port_t port);
uint16_t i686_inw(port_t port);
uint32_t i686_inl(port_t port);

void i686_sti(); // Enable interrupts
void i686_cli(); // Disable interrupts

void i686_iowait();

__attribute__((noreturn)) void i686_panic(void);