/**
 * @file  main.c
 * @brief Minimal demo: 1 ms SysTick timebase + blinking user LED on PD12.
 */
#include <stdint.h>
#include "stm32f407xx.h"
#include "board.h"
#include "system_stm32f4xx.h"

static volatile uint32_t g_tick_ms;

/** Overrides the weak SysTick_Handler alias in startup_stm32f407xx.c. */
void SysTick_Handler(void)
{
    g_tick_ms++;
}

static void delay_ms(uint32_t ms)
{
    uint32_t start = g_tick_ms;
    while ((g_tick_ms - start) < ms) {
        __asm__ volatile("wfi");   /* sleep until the next 1 ms tick */
    }
}

static void led_init(void)
{
    RCC->AHB1ENR |= BOARD_LED_RCC_BIT;
    (void)RCC->AHB1ENR;           /* dummy read after enabling the clock */

    uint32_t pin2 = BOARD_LED_PIN * 2U;
    uint32_t mask = 3U << pin2;

    BOARD_LED_PORT->MODER   = (BOARD_LED_PORT->MODER   & ~mask) | (1U << pin2); /* output */
    BOARD_LED_PORT->OTYPER &= ~(1U << BOARD_LED_PIN);                          /* push-pull */
    BOARD_LED_PORT->OSPEEDR = (BOARD_LED_PORT->OSPEEDR & ~mask) | (1U << pin2); /* medium */
    BOARD_LED_PORT->PUPDR  &= ~mask;                                            /* no pull */
}

static void led_set(uint32_t on)
{
#if BOARD_LED_ACTIVE_HIGH
    BOARD_LED_PORT->BSRR = on ? (1U << BOARD_LED_PIN)
                              : (1U << (BOARD_LED_PIN + 16U));
#else
    BOARD_LED_PORT->BSRR = on ? (1U << (BOARD_LED_PIN + 16U))
                              : (1U << BOARD_LED_PIN);
#endif
}

static void systick_init_1ms(void)
{
    SysTick->LOAD = (SystemCoreClock / 1000U) - 1U;   /* 1 ms @ 168 MHz */
    SysTick->VAL  = 0U;
    SysTick->CTRL = SYSTICK_CTRL_CLKSOURCE
                  | SYSTICK_CTRL_TICKINT
                  | SYSTICK_CTRL_ENABLE;
}

int main(void)
{
    led_init();
    systick_init_1ms();

    for (;;) {
        led_set(1U);
        delay_ms(BOARD_BLINK_MS);
        led_set(0U);
        delay_ms(BOARD_BLINK_MS);
    }
}
