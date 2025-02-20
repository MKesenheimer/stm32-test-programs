#ifndef STM32L0_HAL_H
#define STM32L0_HAL_H

// you probably don't need this from rest of code
//#include "stm32l0_hal_lowlevel.h"

#include <stdint.h>

void init_uart(void);
void putch(char c);
char getch(void);

void trigger_setup(void);
void trigger_low(void);
void trigger_high(void);

void led_error(int);
void led_ok(int);

#endif // STM32L0_HAL_H
