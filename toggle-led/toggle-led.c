#include "hal.h"
#include <stdint.h>
#include <stdlib.h>

#include "stm32l0_hal.h"

uint8_t infinite_loop() {

    // comes on after 3.6ms
    led_ok(1);
    led_error(1);

    while(1) {}
    return 0;
}

int main(void) {
    platform_init();
    infinite_loop();
}
