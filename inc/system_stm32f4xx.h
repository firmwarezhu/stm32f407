/**
 * @file  system_stm32f4xx.h
 * @brief CMSIS-style system clock API (minimal implementation).
 */
#ifndef SYSTEM_STM32F4XX_H
#define SYSTEM_STM32F4XX_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** System clock frequency (Hz) after SystemInit(). Updated by SystemCoreClockUpdate(). */
extern uint32_t SystemCoreClock;

/** Called by Reset_Handler before .data/.bss setup completes. */
void SystemInit(void);

/** Recompute SystemCoreClock from the current RCC registers. */
void SystemCoreClockUpdate(void);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_STM32F4XX_H */
