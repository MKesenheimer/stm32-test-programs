/*
    This file is part of the ChipWhisperer Example Targets
    Copyright (C) 2012-2020 NewAE Technology Inc.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "hal.h"
#include <stdint.h>
#include <stdlib.h>

#include "simpleserial.h"

#include "stm32l0_hal.h"
//#include "stm32l562xx.h"
#include "stm32l0xx_hal_rcc.h"
#include "stm32l0xx_hal_gpio.h"
#include "stm32l0xx_hal_dma.h"
#include "stm32l0xx_hal_uart.h"
#include "stm32l0xx_hal_cryp.h"

uint8_t infinite_loop() {

    // comes on after 3.6ms
    led_ok(1);
    led_error(1);

    //Some fake variable
    volatile uint8_t a = 0;

    /*//External trigger logic
    for (uint32_t b = 0; b < 1024; b++)
        trigger_high();
    trigger_low();*/

    //Should be an infinite loop
    while(a != 2){
    ;
    }

    return 0;
}

// overwrite GPIO setup of PIN PA12 to be able to short this pin with VDD -> Trigger is enabled if supply voltage rises
void pa12_setup(void) {
    GPIO_InitTypeDef GpioInit;
    GpioInit.Pin       = GPIO_PIN_12;
    GpioInit.Mode      = GPIO_MODE_INPUT;
    GpioInit.Pull      = GPIO_PULLDOWN;
    GpioInit.Speed     = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GpioInit);
}

int main(void) {
    platform_init();
    pa12_setup();
    infinite_loop();
}
