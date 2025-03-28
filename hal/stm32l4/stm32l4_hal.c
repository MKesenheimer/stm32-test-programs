#include "stm32l4_hal.h"
#include "stm32l4xx_hal_rcc.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_dma.h"
#include "stm32l4xx_hal_uart.h"
#include "stm32l4xx_hal_cryp.h"

void HAL_IncTick(void);
void SysTick_Handler(void) {
  HAL_IncTick();
}

void _exit(int status) {
    while(1);
}

UART_HandleTypeDef UartHandle;

void platform_init(void) {
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 20U)|(3UL << 22U));  /* set CP10 and CP11 Full Access */
#endif

    // LED Pins init
    __HAL_RCC_GPIOC_CLK_ENABLE(); 
    GPIO_InitTypeDef GpioInit;
    // PC14: Pin 2
    // PC15: Pin 3
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
    // PA9: Pin 19, USART1_TX
    // PA10: Pin 20, USART_RX
    GpioInit.Pin       = GPIO_PIN_9 | GPIO_PIN_10;
    GpioInit.Mode      = GPIO_MODE_AF_PP;
    GpioInit.Pull      = GPIO_PULLUP;
    GpioInit.Speed     = GPIO_SPEED_FREQ_HIGH;
    GpioInit.Alternate = GPIO_AF7_USART1;
    __GPIOA_CLK_ENABLE();
    HAL_GPIO_Init(GPIOA, &GpioInit);

    UartHandle.Instance        = USART1;
    UartHandle.Init.BaudRate   = 115200;
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
    // PA12: Pin 22
    GpioInit.Pin       = GPIO_PIN_12;
    GpioInit.Mode      = GPIO_MODE_OUTPUT_PP;
    GpioInit.Pull      = GPIO_NOPULL;
    GpioInit.Speed     = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GpioInit);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, RESET);
}

void trigger_high(void) {
    // Pin 22
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, SET);
}

void trigger_low(void) {
    // Pin 22
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
    return (char)d;
}

void putch(char c) {
    uint8_t d = c;
    HAL_UART_Transmit(&UartHandle,  &d, 1, 5000);
}