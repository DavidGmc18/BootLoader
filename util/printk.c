#include "printk.h"
#include <stdbool.h>
#include <driver/vga/vga_text.h> // TODO arch should not include driver

static void putc(char c) {
    VGA_putc(c);
}

static void puts(const char* str) {
    while (*str) {
        putc(*str++);
    }
}

static char digits[16] = "0123456789ABCDEF";

static void print_unsigned(unsigned int num, unsigned int radix) {
    if (radix < 2 || radix > 16) {
        puts("BAD_RADIX");
        return;
    }

    if (num == 0) {
        putc('0');
        return;
    }

    char buffer[sizeof(int) * 8];
    int i = 0;
    while (num > 0) {
        buffer[i++] = digits[num % radix];
        num /= radix;
    }

    while (i-- > 0) {
        putc(buffer[i]);
    }
}

static void print_signed(int num, unsigned int radix) {
    if (num < 0) {
        putc('-');
        num = -num;
    }
    print_unsigned((int)num, radix);
}

void vprintk(const char* format, va_list args) {
    bool specifier = false;

    while (*format) {
        if (!specifier) {
            switch (*format) {
                case '%':
                    specifier = true;
                    break;

                default:
                    putc(*format);
                    break;
            }
        } else {
            switch (*format) {
                case 's':
                    puts(va_arg(args, const char*));
                    break;

                case 'c':
                    putc((char)va_arg(args, int));
                    break;

                case 'd':
                case 'i':
                    print_signed(va_arg(args, int), 10);
                    break;

                case 'u':
                    print_unsigned(va_arg(args, unsigned int), 10);
                    break;

                case 'x':
                case 'X':
                    print_unsigned(va_arg(args, unsigned int), 16);
                    break;    
            
                default:
                    putc('%');
                    putc(*format);
                    break;
            }
            specifier = false;
        }
        format++;
    }
}

void printk(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintk(format, args);
    va_end(args);
}