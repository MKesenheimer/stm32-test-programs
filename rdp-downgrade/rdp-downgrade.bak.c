
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
#ifdef STM32L0
    uint8_t rdp_level = READ_BIT(FLASH->OPTR, FLASH_OPTR_RDPROT);
#endif
#ifdef STM32L4
    uint32_t rdp_level = READ_BIT(FLASH->OPTR, FLASH_OPTR_RDP);
#endif
    if ((rdp_level != OB_RDP_LEVEL_0) && (rdp_level != OB_RDP_LEVEL_2)) {
        return (OB_RDP_LEVEL_1);
    } else {
        return rdp_level;
    }
}

// copied from stm32l0xx_hal_flash_ex.c (private function)
static HAL_StatusTypeDef FLASH_OB_ProtectedSectorsConfig(uint32_t Sector, uint32_t NewState) {
    HAL_StatusTypeDef status = HAL_OK;
    uint32_t WRP_Data = 0;
    uint32_t OB_WRP = Sector;
    
    /* Wait for last operation to be completed */
    status = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);
    
    if(status == HAL_OK) {
        /* Clean the error context */
        pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;

        /* Update WRP only if at least 1 selected sector */
        if (OB_WRP != 0x00000000U) {
            if ((OB_WRP & WRP_MASK_LOW) != 0x00000000U) {
                if (NewState != OB_WRPSTATE_DISABLE) {
                    WRP_Data = (uint16_t)(((OB_WRP & WRP_MASK_LOW) | OB->WRP01));
                    OB->WRP01 = (uint32_t)(~(WRP_Data) << 16U) | (WRP_Data);
                } else {
                    WRP_Data = (uint16_t)(~OB_WRP & (WRP_MASK_LOW & OB->WRP01));
                    OB->WRP01 =  (uint32_t)((~WRP_Data) << 16U) | (WRP_Data);
                }
            }
        }
    }
    /* Wait for last operation to be completed */
    status = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);

    /* Return the write protection operation Status */
    return status;      
}

// copied from stm32l0xx_hal_flash_ex.c (private function)
HAL_StatusTypeDef HAL_FLASHEx_OB_SET_PCROP(uint32_t pcrop_config) {
    HAL_StatusTypeDef status = HAL_OK;
    uint16_t tmp1 = 0;
    uint32_t tmp2 = 0;
    uint8_t optiontmp = 0;
    uint16_t optiontmp2 = 0;
    
    status = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);
    
    /* Mask RDP Byte */
    optiontmp =  (uint8_t)(*(__IO uint8_t *)(OB_BASE)); 
    
    /* Update Option Byte */
    optiontmp2 = (uint16_t)(pcrop_config | optiontmp); 
    
    /* calculate the option byte to write */
    tmp1 = (uint16_t)(~(optiontmp2 ));
    tmp2 = (uint32_t)(((uint32_t)((uint32_t)(tmp1) << 16U)) | ((uint32_t)optiontmp2));
           
    /* Clean the error context */
    pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;

    /* program PCRop */
    printf1("rdp value to program: %x\r\n", tmp2);
    //OB->RDP = tmp2;

    /* Wait for last operation to be completed */
    status = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);

    /* Return the Read protection operation Status */
    return status;            
}

#ifdef STM32L0
// copied from stm32l0xx_hal_flash_ex.c (private function)
extern FLASH_ProcessTypeDef pFlash;
static HAL_StatusTypeDef FLASH_OB_RDPConfig(uint8_t OB_RDP) {
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
    //printf1("flash wait for last operation status: %d\r\n", status);

    if(status == HAL_OK) {
        /* Clean the error context */
        pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;

        /* open glitching window */
        led_error(1);

        /* program read protection level */
        OB->RDP = tmp2;

        /* Wait for last operation to be completed */
        status = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);
        //printf1("flash wait for last operation status: %d\r\n", status);

        /* close glitching window */
        led_error(0);
    }

    /* Return the Read protection operation Status */
    return status;
}
#endif

#ifdef STM32L4
static HAL_StatusTypeDef FLASH_OB_RDPConfig(uint32_t RDPLevel) {
    /* init */
    HAL_StatusTypeDef status;

    /* Wait for last operation to be completed */
    status = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

    if(status == HAL_OK) {
        /* Configure the RDP level in the option bytes register */
        MODIFY_REG(FLASH->OPTR, FLASH_OPTR_RDP, RDPLevel);

        /* open glitching window */
        led_error(1);

        /* Set OPTSTRT Bit: start option byte programming */
        SET_BIT(FLASH->CR, FLASH_CR_OPTSTRT);

        /* Wait for last operation to be completed */
        status = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

        /* close glitching window */
        led_error(0);

        /* If the option byte program operation is completed, disable the OPTSTRT Bit */
        CLEAR_BIT(FLASH->CR, FLASH_CR_OPTSTRT);
    }

    /* Return the Read protection operation Status */
    return status;
}
#endif

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
    //led_error(1);
    HAL_StatusTypeDef status3 = FLASH_OB_RDPConfig(rdp_level);
    //led_error(0);
    //led_error(1);
    HAL_StatusTypeDef status4 = HAL_FLASH_Lock();
    //led_error(0);
    __enable_irq();

#if 1
    printf1("flash unlock status: %d\r\n", status1);
    printf1("flash option bytes unlock status: %d\r\n", status2);
    printf1("flash rdp programming status: %d\r\n", status3);
    printf1("flash lock status: %d\r\n", status4);
#endif
}

void flash_erase(void) {
    __disable_irq();

#ifdef STM32L0
    // Define the page and bank for the erase operation
    FLASH_EraseInitTypeDef erase_init;
    uint32_t page_error = 0;
    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.PageAddress = 0x08000000;
    erase_init.NbPages = 32;
#endif
#ifdef STM32L4
    // Define the page and bank for the erase operation
    FLASH_EraseInitTypeDef erase_init;
    uint32_t page_error = 0;
    erase_init.TypeErase = FLASH_TYPEERASE_MASSERASE;
    erase_init.Banks = 1;
#endif

    //led_error(1);
    HAL_StatusTypeDef status1 = HAL_FLASH_Unlock();
    //led_error(0);
    led_error(1);
    HAL_StatusTypeDef status2 = HAL_FLASHEx_Erase(&erase_init, &page_error);
    led_error(0);
    //led_error(1);
    HAL_StatusTypeDef status3 = HAL_FLASH_Lock();
    //led_error(0);
    __enable_irq();

#if 1
    printf1("flash unlock status: %d\r\n", status1);
    printf1("flash erase status: %d\r\n", status2);
    printf1("flash lock status: %d\r\n", status3);
#endif
}

void write_to_flash_address(uint32_t address, uint32_t value) {
    HAL_StatusTypeDef status1 = HAL_FLASH_Unlock();
    HAL_StatusTypeDef status2 = HAL_FLASH_OB_Unlock();
    /*CLEAR_BIT(FLASH->SR, FLASH_FLAG_BSY);
    CLEAR_BIT(FLASH->SR, FLASH_FLAG_EOP);
    CLEAR_BIT(FLASH->SR, FLASH_FLAG_WRPERR);
    CLEAR_BIT(FLASH->SR, FLASH_FLAG_PGAERR);
    //uint32_t wrp01 = (uint32_t)(FLASH->WRPR);*/
    //OB->WRP01 = 0xffff0000;
    //uint32_t wrp01 = (uint32_t)(OB->WRP01);
    //uint32_t rdp = (uint32_t)(OB->RDP);
    
    HAL_StatusTypeDef status3 = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);
    uint32_t read1 = *(__IO uint32_t *)(uint32_t)(address);

    led_error(1);
    //SET_BIT(FLASH->PECR, FLASH_PECR_ERASE);
    //SET_BIT(FLASH->PECR, FLASH_PECR_PROG);
    *(__IO uint32_t *)(uint32_t)(address) = value;
    led_error(0);
    
    HAL_StatusTypeDef status4 = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);

    HAL_StatusTypeDef status5 = HAL_FLASH_Lock();
    HAL_StatusTypeDef status6 = HAL_FLASH_OB_Lock();
    HAL_StatusTypeDef status7 = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);

#if 1
    printf1("flash unlock status: %d\r\n", status1);
    printf1("flash option bytes unlock status: %d\r\n", status2);
    //printf1("write protection value: %x\r\n", wrp01);
    //printf1("protection value: %x\r\n", rdp);
    printf1("flash wait for last operation status: %d\r\n", status3);
    printf1("original value at address 0x%x: 0x%x\r\n", address, read1);
    printf1("flash wait for last operation status: %d\r\n", status4);
    printf1("flash lock status: %d\r\n", status5);
    printf1("flash option bytes lock status: %d\r\n", status6);
    printf1("flash wait for last operation status: %d\r\n", status7);
    uint32_t read2 = *(__IO uint32_t *)(uint32_t)(address);
    printf1("modified value at address 0x%x: 0x%x\r\n", address, read2);
    if (status1 != 0 || status2 != 0 || status3 != 0 || status4 != 0 || status5 != 0 || status6 != 0 || status7 != 0) {
        // error code 0x01: Programming alignment error
        // error code 0x02: Write protection error
        // error code 0x04: Option validity error
        // error code 0x08: 
        // error code 0x10: Read protected error
        // error code 0x20: FLASH Write or Erase operation aborted
        // error code 0x40: FLASH Write operation is done in a not-erased region
        printf1("flash error code: 0x%x\r\n", pFlash.ErrorCode);
    }
#endif
}

int main(void) {
    platform_init();

    // Trigger = A12, Pin 22
    // LED OK = C15: Pin 3
    // LED Error = C14: Pin 2
    // RX = A10, Pin 20
    // TX = A9, Pin 19

    /* uart communication */
    init_uart();
    printf1("\r\nProgram start\r\n");

    /* sleep */
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

#if 0
    // disable WRPROT (OB_WRPSTATE_DISABLE)
    // or enable WRPRPT (OB_WRPSTATE_ENABLE)
    HAL_StatusTypeDef status = FLASH_OB_ProtectedSectorsConfig(OB_WRP_AllPages, OB_WRPSTATE_DISABLE);
    printf1("flash protected sectors config status: %d\r\n", status);
#endif

#if 0
    // disable PCROP (OB_PCROP_DESELECTED)
    // or enable PCROP (OB_PCROP_SELECTED)
    HAL_StatusTypeDef status = HAL_FLASHEx_OB_SET_PCROP(OB_PCROP_DESELECTED);
    printf1("flash disable PCROP status: %d\r\n", status);
#endif

#if 0
    // configure internal clock with ICSCR register (0x40021004)
    // original value of target: 0x0072b087
    // value if fresh MCU:       0x0059B04C
    //write_to_flash_address(0x40022020, 0x00);
    //write_to_flash_address(0x40022080, 0x00);
    write_to_flash_address(0x40021004, 0x0059B04C);
#endif
}
