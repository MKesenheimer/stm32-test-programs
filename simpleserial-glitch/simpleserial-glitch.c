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

uint8_t glitch_loop(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t* in)
{
    volatile uint16_t i, j;
    volatile uint32_t cnt;
    cnt = 0;
    trigger_high();
    for(i=0; i<50; i++){
        for(j=0; j<50; j++){
            cnt++;
            led_ok(0);
            led_error(1);
        }
    }
    trigger_low();
    led_ok(1);
    led_error(0);
    simpleserial_put('r', 4, (uint8_t*)&cnt);
    return (cnt != 2500) ? 0x10 : 0x00;
}

uint8_t glitch_comparison(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t* in)
{
    uint8_t ok = 5;
    trigger_high();
    if (*in == 0xA2){
        ok = 1;
    } else {
        ok = 0;
    }
    trigger_low();
    simpleserial_put('r', 1, (uint8_t*)&ok);
    return 0x00;
}

uint8_t password(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t* pw)
{
    char passwd[] = "touch";
    char passok = 1;
    int cnt;

    trigger_high();

    //Simple test - doesn't check for too-long password!
    for(cnt = 0; cnt < 5; cnt++){
        if (pw[cnt] != passwd[cnt]){
            passok = 0;
        }
    }

    trigger_low();

    simpleserial_put('r', 1, (uint8_t*)&passok);
    return 0x00;
}

uint8_t infinite_loop(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t* in)
{
    led_ok(1);
    led_error(0);

    //Some fake variable
    volatile uint8_t a = 0;

    //External trigger logic
    trigger_high();
    trigger_low();

    //Should be an infinite loop
    while(a != 2){
    ;
    }

    led_error(1);
    led_error(1);
    led_error(1);
    led_error(1);
    led_error(1);
    led_error(1);
    led_error(1);
    led_error(1);
    led_error(1);

    putch('r');
    putch('B');
    putch('R');
    putch('E');
    putch('A');
    putch('K');
    putch('O');
    putch('U');
    putch('T');
    putch('\n');

    led_error(1);
    led_error(1);
    led_error(1);
    led_error(1);
    led_error(1);
    led_error(1);
    led_error(1);
    led_error(1);

    return 0;
}

int main(void)
{
    platform_init();
    init_uart();
    trigger_setup();

    led_ok(1);
    led_error(0);

    /* Device reset detected */
    putch('r');
    putch('R');
    putch('E');
    putch('S');
    putch('E');
    putch('T');
    putch(' ');
    putch(' ');
    putch(' ');
    putch('\n');

    simpleserial_init();
    simpleserial_addcmd('g', 0, glitch_loop);
    simpleserial_addcmd('c', 1, glitch_comparison);
    simpleserial_addcmd(0x01, 5, password);
    simpleserial_addcmd('i', 0, infinite_loop);
    while(1)
        simpleserial_get();
}
