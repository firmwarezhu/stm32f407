/**
 * @file  board.h
 * @brief Board-specific pin mapping. Change this file (not main.c) for your PCB.
 *
 * Default target: STM32F4-Discovery (STM32F407VGT6, HSE 8 MHz, green LED on PD12).
 */
#ifndef BOARD_H
#define BOARD_H

#include "stm32f407xx.h"

/* HSE crystal frequency in Hz. Override from the build with -DHSE_VALUE=... */
#ifndef HSE_VALUE
#define HSE_VALUE 8000000UL
#endif

/* User LED - STM32F4-Discovery: PD12 green, PD13 orange, PD14 red, PD15 blue */
#define BOARD_LED_PORT      GPIOD
#define BOARD_LED_PIN       12U
#define BOARD_LED_RCC_BIT   RCC_AHB1ENR_GPIODEN
#define BOARD_LED_ACTIVE_HIGH 1

/* Blink period (half-period in ms) */
#define BOARD_BLINK_MS      500U

#endif /* BOARD_H */
