
#include "stm32l0_hal.h"
//#include "stm32l562xx.h"
#include "stm32l0xx_hal_rcc.h"
#include "stm32l0xx_hal_gpio.h"
#include "stm32l0xx_hal_dma.h"
#include "stm32l0xx_hal_uart.h"
#include "stm32l0xx_hal_cryp.h"

UART_HandleTypeDef UartHandle;
DMA_HandleTypeDef DmaHandle;

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

// Change system clock to the default clock source
void default_clock() {
    // Because the debugger switches PLL on, we may need to switch
    // back to the HSI oscillator before we can configure the PLL

    // Enable HSI oscillator
    SET_BIT(RCC->CR, RCC_CR_MSION);

    // Wait until MSI oscillator is ready
    while(!READ_BIT(RCC->CR, RCC_CR_MSIRDY)) {}

    // Range 5 = 2.097 MHz
    MODIFY_REG(RCC->ICSCR, RCC_ICSCR_MSIRANGE, RCC_ICSCR_MSIRANGE_5);

    // Switch to MSI oscillator
    MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_MSI);
    
    // Wait until the switch is done
    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_MSI) {}

    // Update variable
    SystemCoreClock = 2097000;

    // Switch the HSI oscillator and PLL off
    CLEAR_BIT(RCC->CR, RCC_CR_HSION);
    CLEAR_BIT(RCC->CR, RCC_CR_PLLON);
}

void platform_init(void) {
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 20U)|(3UL << 22U));  /* set CP10 and CP11 Full Access */
#endif

    //init_clock();
    default_clock();

    // LED Pins init
    __HAL_RCC_GPIOC_CLK_ENABLE(); 
    GPIO_InitTypeDef GpioInit;
    GpioInit.Pin       = GPIO_PIN_14 | GPIO_PIN_15;
    GpioInit.Mode      = GPIO_MODE_OUTPUT_PP;
    GpioInit.Pull      = GPIO_NOPULL;
    GpioInit.Speed     = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &GpioInit);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, RESET);

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
    UartHandle.Init.BaudRate   = 115200;
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits   = UART_STOPBITS_1;
    UartHandle.Init.Parity     = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode       = UART_MODE_TX_RX;
    //UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
    __USART1_CLK_ENABLE();
    HAL_UART_Init(&UartHandle);
}

void init_dma(void) {
    // Example for USART on DMA1_Channel2
    DmaHandle.Instance = DMA1_Channel2;
    DmaHandle.Init.Direction = DMA_MEMORY_TO_PERIPH;
    DmaHandle.Init.PeriphInc = DMA_PINC_DISABLE;
    DmaHandle.Init.MemInc = DMA_MINC_ENABLE;
    DmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    DmaHandle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    DmaHandle.Init.Mode = DMA_NORMAL;
    DmaHandle.Init.Priority = DMA_PRIORITY_LOW;
    __DMA1_CLK_ENABLE();
    HAL_DMA_Init(&DmaHandle);
    __HAL_LINKDMA(&UartHandle, hdmatx, DmaHandle);
}

void trigger_setup(void) {
    GPIO_InitTypeDef GpioInit;
    GpioInit.Pin       = GPIO_PIN_12;
    GpioInit.Mode      = GPIO_MODE_OUTPUT_PP;
    GpioInit.Pull      = GPIO_NOPULL;
    GpioInit.Speed     = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GpioInit);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, RESET);
}

void trigger_high(void) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, SET);
}

void trigger_low(void) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, RESET);
}

void led_error(int val) {
    // Pin 2
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, val);
}

void led_ok(int val) {
    // Pin 3
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, val);
}

char getch(void) {
    uint8_t d;
    while (HAL_UART_Receive(&UartHandle, &d, 1, 50) != HAL_OK) {
        __HAL_UART_CLEAR_OREFLAG(&UartHandle);
    }
    return d;
}

void putch(char c) {
    uint8_t d = c;
    HAL_UART_Transmit(&UartHandle, &d, 1, 5000);
}

void uart_tx_dma(const uint8_t *address, uint16_t size) {
    HAL_UART_Transmit_DMA(&UartHandle, address, size);
}