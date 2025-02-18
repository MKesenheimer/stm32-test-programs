#include <stdarg.h>
#include <string.h>
#include "printf.h"
#include "stm32l0_hal.h"

char *trimwhitespace_unsafe(char *str) {
    char *end;

    // trim leading space
    // move pointer to the first non-space character
    while(isspace((unsigned char)*str)) str++;

    // all spaces?
    if(*str == 0)
        return str;

    // trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // write new null terminator character
    end[1] = '\0';

    return str;
}

uint16_t trimwhitespace(char *out, uint16_t len, const char *str) {
    if(len == 0)
        return 0;

    const char *end;
    uint16_t out_size;

    // trim leading space
    // move pointer to the first non-space character
    while (*str == ' ') str++;

    // all spaces?
    if (*str == 0) {
        *out = 0;
        return 1;
    }

    // trim trailing space
    // move pointer from end of string to the first non-space character
    end = str + strlen(str) - 1;
    while(end > str && (*end == ' ')) end--;
    end++;

    // set output size to minimum of trimmed string length and buffer size minus 1
    out_size = (end - str) < len-1 ? (end - str) : len - 1;

    // copy trimmed string and add null terminator
    memcpy(out, str, out_size);
    out[out_size] = 0;

    return out_size;
}

void printf1(const char *format, ...) {
    va_list args;
    va_start(args, format);

    /*uint16_t sz = 1024;
    char substr[sz];
    const char *formatsub = substr;
    sz = trimwhitespace(formatsub, sz, format);*/
    const char *formatsub = format;

    while (*formatsub) {
        if (*formatsub == '%') {
            formatsub++; // Move to next character
            switch (*formatsub) {
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
                    putch(*formatsub);
                    break;
            }
        } else {
            putch(*formatsub);
        }
        formatsub++;
    }

    va_end(args);
}