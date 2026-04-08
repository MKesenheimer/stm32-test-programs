
#include "stm32g0_hal.h"
#include "stm32g0xx_hal_rcc.h"
#include "stm32g0xx_hal_pwr_ex.h"
#include "stm32g0xx_hal_gpio.h"
#include "stm32g0xx_hal_dma.h"
#include "stm32g0xx_hal_uart.h"
#include "stm32g0xx_hal_cryp.h"
#include "stm32g0xx_hal_flash.h"

UART_HandleTypeDef UartHandle;
DMA_HandleTypeDef DmaHandle;

void Error_Handler(void) {
  while(1) {
    // add here custom error handling code
  }
}

// Change system clock to 32 MHz using internal 16 MHz R/C oscillator
void init_clock() {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
    */
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
    RCC_OscInitStruct.PLL.PLLN = 8;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
    //RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        //Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        //Error_Handler();
    }
}

void platform_init(void) {
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 20U)|(3UL << 22U));  /* set CP10 and CP11 Full Access */
#endif

    init_clock();

    // LED Pins init
    __HAL_RCC_GPIOB_CLK_ENABLE(); 
    GPIO_InitTypeDef GpioInit;
    GpioInit.Pin       = GPIO_PIN_0 | GPIO_PIN_1;
    GpioInit.Mode      = GPIO_MODE_OUTPUT_PP;
    GpioInit.Pull      = GPIO_NOPULL;
    GpioInit.Speed     = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GpioInit);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, RESET);

    //SysTick interrupt will cause power trace noise - it's also re-enabled elsewhere,
    //so we just disable global interrupts. Re-enable this for more interesting work.
    //NVIC_DisableIRQ(SysTick_IRQn);
    __disable_irq();
}

void init_uart(void) {
    GPIO_InitTypeDef GpioInit;
    GpioInit.Pin       = GPIO_PIN_6 | GPIO_PIN_7;
    GpioInit.Mode      = GPIO_MODE_AF_PP;
    GpioInit.Pull      = GPIO_PULLUP;
    GpioInit.Speed     = GPIO_SPEED_FREQ_HIGH;
    GpioInit.Alternate = GPIO_AF4_USART1;
    __GPIOA_CLK_ENABLE();
    HAL_GPIO_Init(GPIOB, &GpioInit);

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
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GpioInit;
    GpioInit.Pin       = GPIO_PIN_8 | GPIO_PIN_9;
    GpioInit.Mode      = GPIO_MODE_OUTPUT_PP;
    GpioInit.Pull      = GPIO_NOPULL;
    GpioInit.Speed     = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GpioInit);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, RESET);
}

void trigger0_high(void) {
    // Pin 21
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, SET);
}

void trigger0_low(void) {
    // Pin 21
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, RESET);
}

void trigger1_high(void) {
    // Pin 22
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, SET);
}

void trigger1_low(void) {
    // Pin 22
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, RESET);
}

void led_error(int val) {
    // Pin 14
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, val);
}

void led_ok(int val) {
    // Pin 15
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, val);
}

void _nop_delay(uint32_t iterations) {
    while (iterations > 0) {
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        iterations--;
    }
}

/* Delay in milliseconds using __NOP() loops.
 * Based on default clock of ~2.1 MHz (MSI Range 5). */
void delay_ms(uint32_t ms) {
    /* ~2.1 MHz clock, each iteration ~10 cycles
     * 2100000 / 10 = 210000 loops per second = 210 per ms */
    _nop_delay(ms * 210);
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