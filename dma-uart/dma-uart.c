
#include "hal.h"
#include <stdint.h>
#include <stdlib.h>

#ifdef STM32L0
#include "stm32l0_hal.h"
#include "stm32l0xx_hal_flash.h"
#include "stm32l0xx_hal_flash_ex.h"
#endif
#ifdef STM32F4
#include "stm32f4_hal.h"
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_flash_ex.h"
#endif
#ifdef STM32L4
#include "stm32l4_hal.h"
#include "stm32l4xx_hal_flash.h"
#include "stm32l4xx_hal_flash_ex.h"
#endif

#include "printf.h"
uint8_t tx_buffer[] = "Hello via DMA!\r\n";

int main(void) {
    platform_init();

    // Trigger = A12, Pin 22
    // LED OK = C15: Pin 3
    // LED Error = C14: Pin 2
    // RX = A10, Pin 20
    // TX = A9, Pin 19

    /* uart communication */
    init_dma();
    init_uart();
    //printf1("\r\nProgram start\r\n");

    /* sleep */
    uint32_t rounds = 5;
    for (int i = 0; i < rounds; ++i) {
        uint32_t cycles = 10000;
        while (cycles--) {
            __NOP();
        }
    }

    //uint32_t address = 0x08000000;
    //uint32_t* tx_buffer = (__IO uint32_t *)(uint32_t)(address);
    led_error(1);
    uart_tx_dma(tx_buffer, sizeof(tx_buffer) - 1);
    led_error(0);

    while (1) {};
}
