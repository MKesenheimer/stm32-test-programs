
#include "stm32l0_hal.h"
//#include "stm32l562xx.h"
#include "stm32l0xx_hal_rcc.h"
#include "stm32l0xx_hal_gpio.h"
#include "stm32l0xx_hal_dma.h"
#include "stm32l0xx_hal_uart.h"
#include "stm32l0xx_hal_cryp.h"

//Just copy these from another project - hopefully ok..
uint32_t SystemCoreClock = 16000000U;

const uint8_t  AHBPrescTable[16] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 1U, 2U, 3U, 4U, 6U, 7U, 8U, 9U};
const uint8_t  APBPrescTable[8] =  {0U, 0U, 0U, 0U, 1U, 2U, 3U, 4U};
const uint32_t MSIRangeTable[16] = {100000U,   200000U,   400000U,   800000U,  1000000U,  2000000U, \
                                    4000000U, 8000000U, 16000000U, 24000000U, 32000000U, 48000000U, \
                                    0U,       0U,       0U,        0U};  /* MISRAC-2012: 0U for unexpected value */

const uint8_t PLLMulTable[9] = {3U, 4U, 6U, 8U, 12U, 16U, 24U, 32U, 48U};

void SystemInit(void) {
    //Init happens higher up
    
}

void SystemCoreClockUpdate(void) {
    ;
}

void HAL_IncTick(void);
void SysTick_Handler(void) {
  HAL_IncTick();
}

void _exit(int status) {
    while(1);
}

UART_HandleTypeDef UartHandle;

// Change system clock to 32 MHz using internal 16 MHz R/C oscillator
void init_clock() {
    // Because the debugger switches PLL on, we may need to switch
    // back to the HSI oscillator before we can configure the PLL

    // Enable HSI oscillator
    SET_BIT(RCC->CR, RCC_CR_HSION);

    // Wait until HSI oscillator is ready
    while(!READ_BIT(RCC->CR, RCC_CR_HSIRDY)) {}

    // Switch to HSI oscillator
    MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_HSI);

    // Wait until the switch is done
    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_HSI) {}

    // Disable the PLL
    CLEAR_BIT(RCC->CR, RCC_CR_PLLON);

    // Wait until the PLL is fully stopped
    while(READ_BIT(RCC->CR, RCC_CR_PLLRDY)) {}
    
    // Flash latency 1 wait state
    SET_BIT(FLASH->ACR, FLASH_ACR_LATENCY);

    // 32 MHz using the 16 MHz HSI oscillator multiply by 4 divide by 2
    WRITE_REG(RCC->CFGR, RCC_CFGR_PLLSRC_HSI + RCC_CFGR_PLLMUL4 + RCC_CFGR_PLLDIV2);

    // Enable PLL
    SET_BIT(RCC->CR, RCC_CR_PLLON);

    // Wait until PLL is ready
    while(!READ_BIT(RCC->CR, RCC_CR_PLLRDY)) {}

    // Select PLL as clock source
    MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL);

    // Update variable
    SystemCoreClock = 32000000;

    // Switch the MSI oscillator off
    CLEAR_BIT(RCC->CR, RCC_CR_MSION);
}

void platform_init(void) {
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 20U)|(3UL << 22U));  /* set CP10 and CP11 Full Access */
#endif

// used for project "STM32L051Voltage Glitching Loop.ipynb"
// TODO: try project "STM32L051Voltage Glitching Loop.ipynb" with init_clock() routine
#if 0
#define USE_INTERNAL_CLK
#ifdef USE_INTERNAL_CLK
     RCC_OscInitTypeDef RCC_OscInitStruct;
     RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
     RCC_OscInitStruct.HSEState       = RCC_HSE_OFF;
     RCC_OscInitStruct.HSIState       = RCC_HSI_ON;
     RCC_OscInitStruct.PLL.PLLSource  = RCC_PLL_NONE;
     HAL_RCC_OscConfig(&RCC_OscInitStruct);

     RCC_ClkInitTypeDef RCC_ClkInitStruct;
     RCC_ClkInitStruct.ClockType      = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
     RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_HSI;
     RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
     RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
     RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
     uint32_t flash_latency = 0;
     HAL_RCC_ClockConfig(&RCC_ClkInitStruct, flash_latency);
#else
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSEState       = RCC_HSE_BYPASS;
    RCC_OscInitStruct.HSIState       = RCC_HSI_OFF;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLL_NONE;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);  
    
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_ClkInitStruct.ClockType      = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_HSE;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_ACR_LATENCY_5WS);
    for (volatile int i = 0; i < 10000; i++); //firmware doesn't work unless this is here for some reason
#endif
#endif

    // used for project "STM32L051Voltage Glitching Read-Out Protection.ipynb"
#if 1
    init_clock();
#endif

    // LED Pins init
    __HAL_RCC_GPIOC_CLK_ENABLE(); 
    GPIO_InitTypeDef GpioInit;
    GpioInit.Pin       = GPIO_PIN_14 | GPIO_PIN_15;
    GpioInit.Mode      = GPIO_MODE_OUTPUT_PP;
    GpioInit.Pull      = GPIO_NOPULL;
    GpioInit.Speed     = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &GpioInit);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, SET);

    //SysTick interrupt will cause power trace noise - it's also re-enabled elsewhere,
    //so we just disable global interrupts. Re-enable this for more interesting work.
    //NVIC_DisableIRQ(SysTick_IRQn);
    __disable_irq();
}

void init_uart(void) {
    GPIO_InitTypeDef GpioInit;
    GpioInit.Pin       = GPIO_PIN_9 | GPIO_PIN_10;
    GpioInit.Mode      = GPIO_MODE_AF_PP;
    GpioInit.Pull      = GPIO_PULLUP;
    GpioInit.Speed     = GPIO_SPEED_FREQ_HIGH;
    GpioInit.Alternate = GPIO_AF4_USART1;
    __GPIOA_CLK_ENABLE();
    HAL_GPIO_Init(GPIOA, &GpioInit);

    UartHandle.Instance        = USART1;
  #if SS_VER==SS_VER_2_0
  UartHandle.Init.BaudRate   = 230400;
  #else
  UartHandle.Init.BaudRate   = 38400;
  #endif
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits   = UART_STOPBITS_1;
    UartHandle.Init.Parity     = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode       = UART_MODE_TX_RX;
    __USART1_CLK_ENABLE();
    HAL_UART_Init(&UartHandle);
}

void trigger_setup(void) {
    GPIO_InitTypeDef GpioInit;
    GpioInit.Pin       = GPIO_PIN_12;
    GpioInit.Mode      = GPIO_MODE_OUTPUT_PP;
    GpioInit.Pull      = GPIO_NOPULL;
    GpioInit.Speed     = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GpioInit);
}

void trigger_high(void) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, SET);
}

void trigger_low(void) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, RESET);
}

void led_error(int val) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, val);
}

void led_ok(int val) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, val);
}

char getch(void) {
    uint8_t d;
    while (HAL_UART_Receive(&UartHandle, &d, 1, 50) != HAL_OK)
    USART1->ICR |= (1 << 3);
    return d;
}

void putch(char c) {
    uint8_t d  = c;
    HAL_UART_Transmit(&UartHandle,  &d, 1, 5000);
}