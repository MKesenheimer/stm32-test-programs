
#include "hal.h"
#include <stdint.h>
#include <stdlib.h>

#include "stm32l0_hal.h"
#include "stm32l0xx_hal_flash_ex.h"
#include "printf.h"

HAL_StatusTypeDef erase_flash(void) {
    // Unlock the Flash to enable the flash control register access
    HAL_StatusTypeDef status = HAL_FLASH_Unlock();
    if (status != HAL_OK) {
        // Unlock failed, handle accordingly
        return status;
    }

    // Define the page and bank for the erase operation
    FLASH_EraseInitTypeDef erase_init;
    uint32_t page_error = 0;
    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.PageAddress = 0x08000000;
    erase_init.NbPages = 1;

    // Erase the page
    status = HAL_FLASHEx_Erase(&erase_init, &page_error);
    if (status != HAL_OK) {
        // Erase failed, handle accordingly
        HAL_FLASH_Lock();  // Make sure to lock the flash again before returning
        return status;
    }

    HAL_FLASH_Lock();  // Lock the Flash to disable the flash control register access
    return HAL_OK;
}

uint8_t infinite_loop() {

    // comes on after 3.6ms
    led_ok(1);
    led_error(0);

    HAL_StatusTypeDef status = erase_flash();

    if (status != HAL_OK) {
        led_error(1);
    }

    while (1);
    return 0;
}

int main(void) {
    platform_init();
    // RX = A10, Pin 20
    // TX = A9, Pin 19
    init_uart();
    putch('c');
    printf("test\n");
    //infinite_loop();
}
