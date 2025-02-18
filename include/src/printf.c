#include <stdarg.h>
#include "printf.h"
#include "stm32l0_hal.h"

void printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    while (*format) {
        
        if (*format == '%') {
            format++; // Move to next character
            switch (*format) {
                case 'c': { // Character
                    char c = (char)va_arg(args, int);
                    putch(c);
                    break;
                }
                case 's': { // String
                    char *str = va_arg(args, char*);
                    while (*str) {
                        putch(*str++);
                    }
                    break;
                }
                case 'd': { // Integer
                    int num = va_arg(args, int);
                    if (num < 0) {
                        putch('-');
                        num = -num;
                    }
                    char buf[10]; // Buffer for integer to string conversion
                    int i = 0;
                    if (num == 0) {
                        putch('0');
                    } else {
                        while (num > 0) {
                            buf[i++] = (num % 10) + '0';
                            num /= 10;
                        }
                        while (i--) {
                            putch(buf[i]);
                        }
                    }
                    break;
                }
                default:
                    putch('%');
                    putch(*format);
                    break;
            }
        } else {
            putch(*format);
        }
        format++;
    }

    va_end(args);
}