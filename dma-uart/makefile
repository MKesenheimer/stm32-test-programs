# Hey Emacs, this is a -*- makefile -*-
#----------------------------------------------------------------------------
#
# Makefile for program to toggle a led
#
#----------------------------------------------------------------------------
# On command line:
#
# make all = Make software.
#
# make clean = Clean out built project files.
#
# make coff = Convert ELF to AVR COFF.
#
# make extcoff = Convert ELF to AVR Extended COFF.
#
# make program = Download the hex file to the device, using avrdude.
#                Please customize the avrdude settings below first!
#
# make debug = Start either simulavr or avarice as specified for debugging,
#              with avr-gdb or avr-insight as the front end for debugging.
#
# make filename.s = Just compile filename.c into the assembler code only.
#
# make filename.i = Create a preprocessed source file for use in submitting
#                   bug reports to the GCC project.
#
# To rebuild project do "make clean" then "make all".
#----------------------------------------------------------------------------

# choose platform
# STM32L051
FAMILY = STM32L0
CHIP = STM32L051

# STM32L422
#FAMILY = STM32L4
#CHIP = STM32L422

# Target file name (without extension).
# This is the name of the compiled .hex file.
TARGET = dma-uart
EXECTYPE = RAM
CDEFS += -DUSER_VECT_TAB_ADDRESS -DVECT_TAB_SRAM
CPPDEFS += -DUSER_VECT_TAB_ADDRESS -DVECT_TAB_SRAM

# List C source files here.
# Header files (.h) are automatically pulled in.
SRC += dma-uart.c printf.c
ifeq ($(FAMILY),STM32L0)
	SRC += stm32l0xx_hal_dma.c
else ifeq ($(FAMILY),STM32L4)
	OPENOCD_CONFIG = target/stm32l4x.cfg
	SRC += stm32l4xx_hal_dma.c
endif

# -----------------------------------------------------------------------------

FIRMWAREPATH = ../.
include $(FIRMWAREPATH)/Makefile.inc

