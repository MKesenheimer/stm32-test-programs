
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

// copied from stm32l0xx_hal_flash_ex.c (private function)
static uint8_t FLASH_OB_GetRDP(void) {
    uint8_t rdp_level = READ_BIT(FLASH->OPTR, FLASH_OPTR_RDPROT);

    if ((rdp_level != OB_RDP_LEVEL_0) && (rdp_level != OB_RDP_LEVEL_2)) {
        return (OB_RDP_LEVEL_1);
    } else {
        return rdp_level;
    }
}

// copied from stm32l0xx_hal_flash_ex.c (private function)
extern FLASH_ProcessTypeDef pFlash;
static HAL_StatusTypeDef FLASH_OB_RDPConfig(uint8_t OB_RDP) {
    /* open glitching window */
    //led_error(1);

    /* init */
    HAL_StatusTypeDef status = HAL_OK;
    uint32_t tmp1 = 0U, tmp2 = 0U, tmp3 = 0U;
    
    tmp1 = (uint32_t)(OB->RDP & FLASH_OPTR_RDPROT);
  
#if defined(FLASH_OPTR_WPRMOD)
    /* Mask WPRMOD bit */
    tmp3 = (uint32_t)(OB->RDP & FLASH_OPTR_WPRMOD);
#endif

    /* calculate the option byte to write */
    tmp1 = (~((uint32_t)(OB_RDP | tmp3)));
    tmp2 = (uint32_t)(((uint32_t)((uint32_t)(tmp1) << 16U)) | ((uint32_t)(OB_RDP | tmp3)));

    /* Wait for last operation to be completed */
    status = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);

    if(status == HAL_OK) {
        /* Clean the error context */
        pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;

        /* program read protection level */
        OB->RDP = tmp2;

        /* Wait for last operation to be completed */
        status = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);
    }

    /* close glitching window */
    //led_error(0);

    /* Return the Read protection operation Status */
    return status;
}

uint8_t get_rdp_level() {
    uint8_t rdp_level = FLASH_OB_GetRDP();
    
#if 0
    if (rdp_level == OB_RDP_LEVEL_0) {
        printf1("No RDP set.\r\n");
    } else if (rdp_level == OB_RDP_LEVEL_1) {
        printf1("RDP level 1 set.\r\n");
    } else if (rdp_level == OB_RDP_LEVEL_2) {
        printf1("RDP level 2 set.\r\n");
    }
#endif

    return rdp_level;
}

void flash_set_rdp(uint8_t rdp_level) {
    __disable_irq();
    //led_error(1);
    HAL_StatusTypeDef status1 = HAL_FLASH_Unlock();
    //led_error(0);
    //led_error(1);
    HAL_StatusTypeDef status2 = HAL_FLASH_OB_Unlock();
    //led_error(0);
    led_error(1);
    HAL_StatusTypeDef status3 = FLASH_OB_RDPConfig(rdp_level);
    //led_error(0);
    //led_error(1);
    HAL_StatusTypeDef status4 = HAL_FLASH_Lock();
    led_error(0);
    __enable_irq();

#if 0
    printf1("flash unlock status: %d\r\n", status1);
    printf1("flash option bytes unlock status: %d\r\n", status2);
    printf1("flash rdp programming status: %d\r\n", status);
#endif
}

void flash_erase(void) {
    __disable_irq();
    FLASH_EraseInitTypeDef erase_init;
    uint32_t page_error = 0;
    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.PageAddress = 0x08000000;
    erase_init.NbPages = 32;

    //led_error(1);
    HAL_StatusTypeDef status1 = HAL_FLASH_Unlock();
    //led_error(0);
    led_error(1);
    HAL_StatusTypeDef status2 = HAL_FLASHEx_Erase(&erase_init, &page_error);
    //led_error(0);
    //led_error(1);
    HAL_StatusTypeDef status3 = HAL_FLASH_Lock();
    led_error(0);
    __enable_irq();
}

int main(void) {
    platform_init();

    // RX = A10, Pin 20
    // TX = A9, Pin 19
    //init_uart();

    uint32_t rounds = 5;
    for (int i = 0; i < rounds; ++i) {
        uint32_t cycles = 10000;
        while (cycles--) {
            __NOP();
        }
    }

#if 1
    uint8_t rdp_level = get_rdp_level();
    if (rdp_level != OB_RDP_LEVEL_0) {
        flash_set_rdp(OB_RDP_LEVEL_0);
    }
#endif

#if 0
    flash_erase();
#endif

    while (1) {}
}
