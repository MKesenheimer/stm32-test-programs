#include "hal.h"
#include <stdint.h>
#include <stdlib.h>

#ifdef STM32L0
#include "stm32l0_hal.h"
#endif
#ifdef STM32F4
#include "stm32f4_hal.h"
#endif
#ifdef STM32L4
#include "stm32l4_hal.h"
#endif

uint8_t infinite_loop() {

    // comes on after 3.6ms
    led_ok(1);
    led_error(1);

    // some fake variable
    volatile uint8_t a = 0;

    // external trigger logic
    for (uint32_t b = 0; b < 1024; b++)
        trigger_high();
    trigger_low();

    // should be an infinite loop
    while(a != 2) {}

    return 0;
}

int main(void) {
    platform_init();
    trigger_setup();
    infinite_loop();
}
