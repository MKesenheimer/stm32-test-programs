
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

HAL_StatusTypeDef erase_flash(void) {
    // Unlock the Flash to enable the flash control register access
    HAL_StatusTypeDef status = HAL_FLASH_Unlock();
    if (status != HAL_OK) {
        // Unlock failed, handle accordingly
        return status;
    }

#ifdef STM32L0
    // Define the page and bank for the erase operation
    FLASH_EraseInitTypeDef erase_init;
    uint32_t page_error = 0;
    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.PageAddress = 0x08000000;
    erase_init.NbPages = 1;
#endif
#ifdef STM32L4
    // Define the page and bank for the erase operation
    FLASH_EraseInitTypeDef erase_init;
    uint32_t page_error = 0;
    erase_init.TypeErase = FLASH_TYPEERASE_MASSERASE;
#endif

    // Erase the page
    status = HAL_FLASHEx_Erase(&erase_init, &page_error);
    if (status != HAL_OK) {
        // Erase failed, handle accordingly
        HAL_FLASH_Lock();  // Make sure to lock the flash again before returning
        return status;
    }

    // Lock the Flash to disable the flash control register access
    HAL_FLASH_Lock();
    return HAL_OK;
}

uint8_t perform_flash_erase() {
    HAL_StatusTypeDef status = erase_flash();
    printf1("status of erase_flash: %d\r\n", status);

    return status;
}

int main(void) {
    platform_init();
    // RX = A10, Pin 20
    // TX = A9, Pin 19
    init_uart();

    // toggle status leds
    led_ok(0);
    led_error(0);

    printf1("executing flash-erase\r\n");
    perform_flash_erase();
    while (1);
}
