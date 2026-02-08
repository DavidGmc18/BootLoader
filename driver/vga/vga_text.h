#pragma once

#include <stdint.h>
#include <stddef.h>

void VGA_Initialize(uint16_t ScreenWidth, uint16_t ScreenHeight, uint8_t* ScreenBuffer);

void VGA_setcursor(int x, int y);

void VGA_clrscr();

void VGA_putchr(uint16_t x, uint16_t y, char ch);
void VGA_putcolor(uint16_t x, uint16_t y, uint8_t color);
char VGA_getchr(uint16_t x, uint16_t y);
uint8_t VGA_getcolor(uint16_t x, uint16_t y);

void VGA_putc(char ch);
void VGA_putn(const char* str, size_t size);
void VGA_puts(const char* str);

void VGA_set_color(uint8_t color);

void VGA_get_cursor(uint8_t* x, uint8_t* y);