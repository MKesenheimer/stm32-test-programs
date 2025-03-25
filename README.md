# Prerequisites

```bash
arm-none-eabi-gcc
arm-none-eabi-newlib
```

## adding new parts

For example, adding the STM32F4

```bash
git submodule add https://github.com/STMicroelectronics/stm32f4xx-hal-driver.git hal/stm32f4/stm32f4
git submodule add https://github.com/STMicroelectronics/cmsis-device-f4.git hal/stm32f4/cmsis
```

Copy a template Makefile from `hal`, rename and modify it:

```bash
cp hal/Makefile.stm32l0 hal/Makefile.stm32f4
vim hal/Makefile.stm32f4
```