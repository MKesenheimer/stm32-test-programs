# Prerequisites

```bash
arm-none-eabi-gcc
arm-none-eabi-newlib
```

## adding new chips

For example, adding the STM32F4:

```bash
git submodule add https://github.com/STMicroelectronics/stm32f4xx-hal-driver.git hal/stm32f4/stm32f4
git submodule add https://github.com/STMicroelectronics/cmsis-device-f4.git hal/stm32f4/cmsis
```

- Copy a template Makefile from `hal/stm32l0`, rename and modify it. Pay attention to the variable `MCU_FLAGS`.

```bash
cp hal/stm32l0/Makefile.stm32l0 hal/stm32f4/Makefile.stm32f4
vim hal/stm32f4/Makefile.stm32f4
```

- Copy a `LinkerScript.ld` from the suitable `STM32CubeF4` repository, for example:
```bash
cd Desktop
git clone https://github.com/STMicroelectronics/STM32CubeF4.git
cp ./Projects/STM32F429I-Discovery/Templates_LL/STM32CubeIDE/STM32F429ZITX_FLASH.ld $(stm32-test-programs)/hal/stm32f4/LinkerScript.ld
```

- Copy and modify the linker script if needed, for example for RAM execution. Remove the `(READONLY)` keywords if needed.

- Copy a `stm32f4xx_hal_conf.h ` file from the suitable `STM32CubeF4` repository, for example:
```bash
cd Desktop/STM32CubeF4
find . -iname "*hal_conf.h" | grep Template
cp ./Projects/STM32F429I-Discovery/Templates/Inc/stm32f4xx_hal_conf.h $(stm32-test-programs)/hal/stm32f4
```

- Copy the following files and adapt as needed:

```bash
hal/stm32l0/stm32l0_hal.c
hal/stm32l0/stm32l0_hal.h
```