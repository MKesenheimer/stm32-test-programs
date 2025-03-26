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
    led_error(0);

    while(1) {}
    return 0;
}

int main(void) {
    platform_init();
    infinite_loop();
}
