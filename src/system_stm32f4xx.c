/**
 * @file  system_stm32f4xx.c
 * @brief Clock tree setup: HSE -> PLL -> SYSCLK 168 MHz (APB1 42 / APB2 84 MHz).
 *
 * Deliberately written as plain register accesses so it mirrors the boot
 * sequence described in docs/boot-sequence.md and the boot-process infographic.
 * If the HSE crystal does not start, the code falls back to HSI 16 MHz instead
 * of hanging forever (a very common bring-up failure on custom boards).
 */
#include "stm32f407xx.h"
#include "board.h"
#include "system_stm32f4xx.h"

uint32_t SystemCoreClock = 16000000UL;

/* PLL: (HSE / PLL_M) * PLL_N / PLL_P = (8 MHz / 8) * 336 / 2 = 168 MHz.
   PLL_M is derived from HSE_VALUE so 8/12/16/25 MHz crystals all work. */
#define PLL_M   (HSE_VALUE / 1000000UL)
#define PLL_N   336U
#define PLL_P   0U      /* 0b00 = /2 */
#define PLL_Q   7U      /* 336 / 7 = 48 MHz for USB OTG FS / SDIO / RNG */

#define FLAG_TIMEOUT  0x10000UL

static uint32_t wait_flag(const volatile uint32_t *reg, uint32_t mask, uint32_t expected)
{
    uint32_t timeout = FLAG_TIMEOUT;
    while (((*reg & mask) != expected) && (--timeout != 0U)) {
        /* busy wait */
    }
    return ((*reg & mask) == expected) ? 1U : 0U;
}

void SystemInit(void)
{
    /* Enable CP10/CP11 full access for the FPU (must precede any FP code). */
    SCB->CPACR |= SCB_CPACR_CP10_CP11_FULL;
    __asm__ volatile("dsb" ::: "memory");
    __asm__ volatile("isb" ::: "memory");

    /* Reset the clock tree to a known state: HSI on, PLL off, prescalers /1. */
    RCC->CR |= RCC_CR_HSION;
    (void)wait_flag(&RCC->CR, RCC_CR_HSIRDY, RCC_CR_HSIRDY);
    RCC->CFGR = 0U;
    RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_CSSON | RCC_CR_HSEBYP | RCC_CR_PLLON);
    (void)wait_flag(&RCC->CR, RCC_CR_PLLRDY, 0U);

    /* Bring up HSE. On timeout keep running from HSI 16 MHz. */
    RCC->CR |= RCC_CR_HSEON;
    if (wait_flag(&RCC->CR, RCC_CR_HSERDY, RCC_CR_HSERDY) == 0U) {
        SCB->VTOR = FLASH_BASE;
        SystemCoreClockUpdate();
        return;
    }

    /* 168 MHz requires voltage scale 1 (RM0090, PWR_CR.VOS). */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    (void)RCC->APB1ENR;                 /* dummy read: let the clock settle */
    PWR->CR = (PWR->CR & ~PWR_CR_VOS_MASK) | PWR_CR_VOS_SCALE1;

    /* AHB /1, APB1 /4 (42 MHz max), APB2 /2 (84 MHz max). */
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV4 | RCC_CFGR_PPRE2_DIV2;

    /* Configure the main PLL with HSE as source. */
    RCC->PLLCFGR = (PLL_M << RCC_PLLCFGR_PLLM_Pos)
                 | (PLL_N << RCC_PLLCFGR_PLLN_Pos)
                 | (PLL_P << RCC_PLLCFGR_PLLP_Pos)
                 | RCC_PLLCFGR_PLLSRC_HSE
                 | (PLL_Q << RCC_PLLCFGR_PLLQ_Pos);

    /* Raise flash wait states BEFORE raising the clock: 5 WS @ 168 MHz / 3.3 V. */
    FLASH->ACR = FLASH_ACR_LATENCY_5WS
               | FLASH_ACR_PRFTEN
               | FLASH_ACR_ICEN
               | FLASH_ACR_DCEN;

    RCC->CR |= RCC_CR_PLLON;
    if (wait_flag(&RCC->CR, RCC_CR_PLLRDY, RCC_CR_PLLRDY) != 0U) {
        RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_MASK) | RCC_CFGR_SW_PLL;
        (void)wait_flag(&RCC->CFGR, RCC_CFGR_SWS_MASK, RCC_CFGR_SWS_PLL);
    }

    /* Point VTOR at the vector table in flash. Required when a bootloader,
       or any later vector-table relocation, is involved. */
    SCB->VTOR = FLASH_BASE;
    SystemCoreClockUpdate();
}

void SystemCoreClockUpdate(void)
{
    static const uint16_t ahb_div[16] = {
        1, 1, 1, 1, 1, 1, 1, 1, 2, 4, 8, 16, 64, 128, 256, 512
    };
    uint32_t sws = RCC->CFGR & RCC_CFGR_SWS_MASK;
    uint32_t sysclk;

    if (sws == RCC_CFGR_SWS_HSI) {
        sysclk = 16000000UL;
    } else if (sws == RCC_CFGR_SWS_HSE) {
        sysclk = HSE_VALUE;
    } else {
        uint32_t pllcfgr = RCC->PLLCFGR;
        uint32_t pllm = pllcfgr & 0x3FUL;
        uint32_t plln = (pllcfgr >> 6) & 0x1FFUL;
        uint32_t pllp = (((pllcfgr >> 16) & 3UL) + 1UL) * 2UL;
        uint32_t pllsrc = (pllcfgr & RCC_PLLCFGR_PLLSRC_HSE) ? HSE_VALUE : 16000000UL;
        sysclk = (pllm != 0U) ? (pllsrc / pllm) * plln / pllp : 16000000UL;
    }

    uint32_t hpre = (RCC->CFGR >> 4) & 0xFUL;
    SystemCoreClock = sysclk / ahb_div[hpre];
}
