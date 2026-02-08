#pragma once

#include <stdarg.h>

void vprintk(const char* format, va_list args);

void printk(const char* format, ...);