#pragma once
#include <stdint.h>

void init_uart(void);
void putch(char c);
void uart_tx_dma(const uint8_t *address, uint16_t size);
char getch(void);

void trigger_setup(void);
void trigger0_low(void);
void trigger0_high(void);
void trigger0_low(void);
void trigger0_high(void);

void led_error(int);
void led_ok(int);
