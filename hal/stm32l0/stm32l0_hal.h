#pragma once
#include <stdint.h>

void init_uart(void);
void putch(char c);
char getch(void);

void trigger_setup(void);
void trigger_low(void);
void trigger_high(void);

void led_error(int);
void led_ok(int);
