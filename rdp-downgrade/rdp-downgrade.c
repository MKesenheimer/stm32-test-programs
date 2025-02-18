
#include "hal.h"
#include <stdint.h>
#include <stdlib.h>

#include "stm32l0_hal.h"
#include "stm32l0xx_hal_flash.h"
#include "stm32l0xx_hal_flash_ex.h"
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
        led_error(1);
        status = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);
        led_error(0);
    }

    /* Return the Read protection operation Status */
    return status;
}

uint8_t print_rdp_level() {
    uint8_t rdp_level = FLASH_OB_GetRDP();
    
    if (rdp_level == OB_RDP_LEVEL_0) {
        printf1("No RDP set.\r\n");
    } else if (rdp_level == OB_RDP_LEVEL_1) {
        printf1("RDP level 1 set.\r\n");
    } else if (rdp_level == OB_RDP_LEVEL_2) {
        printf1("RDP level 2 set.\r\n");
    }

    return rdp_level;
}

void flash_set_rdp(uint8_t rdp_level) {
    HAL_StatusTypeDef status = HAL_FLASH_Unlock();
    printf1("flash unlock status: %d\r\n", status);
    status = HAL_FLASH_OB_Unlock();
    printf1("flash option bytes unlock status: %d\r\n", status);

    status = FLASH_OB_RDPConfig(OB_RDP_LEVEL_0);
    printf1("flash rdp programming status: %d\r\n", status);
}

int main(void) {
    platform_init();
    // RX = A10, Pin 20
    // TX = A9, Pin 19
    init_uart();

    // status leds
    led_ok(0);
    led_error(0);

    uint8_t rdp_level = print_rdp_level();
    if (rdp_level != OB_RDP_LEVEL_0)
        flash_set_rdp(0x00);

    while (1);
}
