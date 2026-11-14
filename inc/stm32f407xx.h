/**
 * @file  stm32f407xx.h
 * @brief Minimal, dependency-free register definitions for STM32F407 (RM0090).
 *
 * This is NOT the full CMSIS device header. It intentionally covers only the
 * peripherals used by this template. When the project grows, swap in ST's
 * official stm32f407xx.h + core_cm4.h from the CMSIS device pack / CubeF4
 * (see README.md, "Upgrading to HAL / CMSIS").
 */
#ifndef STM32F407XX_H
#define STM32F407XX_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------- memory / peripheral base addresses -------------------- */
#define FLASH_BASE       0x08000000UL   /* 1 MB on STM32F407VG */
#define SRAM_BASE        0x20000000UL   /* 128 KB (112 KB SRAM1 + 16 KB SRAM2) */
#define CCMRAM_BASE      0x10000000UL   /* 64 KB, CPU only, NOT DMA accessible */
#define PERIPH_BASE      0x40000000UL

#define PWR_BASE         (PERIPH_BASE + 0x00007000UL)
#define SYSCFG_BASE      (PERIPH_BASE + 0x00013800UL)
#define GPIOA_BASE       (PERIPH_BASE + 0x00020000UL)
#define GPIOB_BASE       (PERIPH_BASE + 0x00020400UL)
#define GPIOC_BASE       (PERIPH_BASE + 0x00020800UL)
#define GPIOD_BASE       (PERIPH_BASE + 0x00020C00UL)
#define GPIOE_BASE       (PERIPH_BASE + 0x00021000UL)
#define GPIOF_BASE       (PERIPH_BASE + 0x00021400UL)
#define GPIOG_BASE       (PERIPH_BASE + 0x00021800UL)
#define GPIOH_BASE       (PERIPH_BASE + 0x00021C00UL)
#define GPIOI_BASE       (PERIPH_BASE + 0x00022000UL)
#define RCC_BASE         (PERIPH_BASE + 0x00023800UL)
#define FLASH_REG_BASE   (PERIPH_BASE + 0x00023C00UL)

#define SYSTICK_BASE     0xE000E010UL
#define NVIC_BASE        0xE000E100UL
#define SCB_BASE         0xE000ED00UL

/* ------------------------------ peripherals ------------------------------ */
typedef struct {
    volatile uint32_t CR;       /* 0x00 power control */
    volatile uint32_t CSR;      /* 0x04 power control/status */
} PWR_TypeDef;

typedef struct {
    volatile uint32_t ACR;      /* 0x00 flash access control */
    volatile uint32_t KEYR;     /* 0x04 */
    volatile uint32_t OPTKEYR;  /* 0x08 */
    volatile uint32_t SR;       /* 0x0C */
    volatile uint32_t CR;       /* 0x10 */
    volatile uint32_t OPTCR;    /* 0x14 */
    volatile uint32_t OPTCR1;   /* 0x18 (reserved on F405/407) */
} FLASH_TypeDef;

typedef struct {
    volatile uint32_t CR;           /* 0x00 */
    volatile uint32_t PLLCFGR;      /* 0x04 */
    volatile uint32_t CFGR;         /* 0x08 */
    volatile uint32_t CIR;          /* 0x0C */
    volatile uint32_t AHB1RSTR;     /* 0x10 */
    volatile uint32_t AHB2RSTR;     /* 0x14 */
    volatile uint32_t AHB3RSTR;     /* 0x18 */
    uint32_t          RESERVED0;    /* 0x1C */
    volatile uint32_t APB1RSTR;     /* 0x20 */
    volatile uint32_t APB2RSTR;     /* 0x24 */
    uint32_t          RESERVED1[2]; /* 0x28 - 0x2C */
    volatile uint32_t AHB1ENR;      /* 0x30 */
    volatile uint32_t AHB2ENR;      /* 0x34 */
    volatile uint32_t AHB3ENR;      /* 0x38 */
    uint32_t          RESERVED2;    /* 0x3C */
    volatile uint32_t APB1ENR;      /* 0x40 */
    volatile uint32_t APB2ENR;      /* 0x44 */
    uint32_t          RESERVED3[2]; /* 0x48 - 0x4C */
    volatile uint32_t AHB1LPENR;    /* 0x50 */
    volatile uint32_t AHB2LPENR;    /* 0x54 */
    volatile uint32_t AHB3LPENR;    /* 0x58 */
    uint32_t          RESERVED4;    /* 0x5C */
    volatile uint32_t APB1LPENR;    /* 0x60 */
    volatile uint32_t APB2LPENR;    /* 0x64 */
    uint32_t          RESERVED5[2]; /* 0x68 - 0x6C */
    volatile uint32_t BDCR;         /* 0x70 */
    volatile uint32_t CSR;          /* 0x74 */
    uint32_t          RESERVED6[2]; /* 0x78 - 0x7C */
    volatile uint32_t SSCGR;        /* 0x80 */
    volatile uint32_t PLLI2SCFGR;   /* 0x84 */
    uint32_t          RESERVED7;    /* 0x88 */
    volatile uint32_t DCKCFGR;      /* 0x8C */
} RCC_TypeDef;

typedef struct {
    volatile uint32_t MODER;    /* 0x00 */
    volatile uint32_t OTYPER;   /* 0x04 */
    volatile uint32_t OSPEEDR;  /* 0x08 */
    volatile uint32_t PUPDR;    /* 0x0C */
    volatile uint32_t IDR;      /* 0x10 */
    volatile uint32_t ODR;      /* 0x14 */
    volatile uint32_t BSRR;     /* 0x18 */
    volatile uint32_t LCKR;     /* 0x1C */
    volatile uint32_t AFR[2];   /* 0x20 / 0x24 */
} GPIO_TypeDef;

typedef struct {
    volatile uint32_t CTRL;     /* 0x00 */
    volatile uint32_t LOAD;     /* 0x04 */
    volatile uint32_t VAL;      /* 0x08 */
    volatile uint32_t CALIB;    /* 0x0C */
} SysTick_TypeDef;

typedef struct {
    volatile uint32_t ISER[8];  /* 0x000 */
    uint32_t RESERVED0[24];
    volatile uint32_t ICER[8];  /* 0x080 */
    uint32_t RESERVED1[24];
    volatile uint32_t ISPR[8];  /* 0x100 */
    uint32_t RESERVED2[24];
    volatile uint32_t ICPR[8];  /* 0x180 */
    uint32_t RESERVED3[24];
    volatile uint32_t IABR[8];  /* 0x200 */
    uint32_t RESERVED4[56];
    volatile uint8_t  IP[240];  /* 0x300 */
} NVIC_TypeDef;

typedef struct {
    volatile uint32_t CPUID;        /* 0x000 */
    volatile uint32_t ICSR;         /* 0x004 */
    volatile uint32_t VTOR;         /* 0x008 */
    volatile uint32_t AIRCR;        /* 0x00C */
    volatile uint32_t SCR;          /* 0x010 */
    volatile uint32_t CCR;          /* 0x014 */
    volatile uint8_t  SHP[12];      /* 0x018 */
    volatile uint32_t SHCSR;        /* 0x024 */
    volatile uint32_t CFSR;         /* 0x028 */
    volatile uint32_t HFSR;         /* 0x02C */
    volatile uint32_t DFSR;         /* 0x030 */
    volatile uint32_t MMFAR;        /* 0x034 */
    volatile uint32_t BFAR;         /* 0x038 */
    volatile uint32_t AFSR;         /* 0x03C */
    volatile uint32_t ID_PFR[2];    /* 0x040 */
    volatile uint32_t ID_DFR;       /* 0x048 */
    volatile uint32_t ID_AFR;       /* 0x04C */
    volatile uint32_t ID_MFR[4];    /* 0x050 */
    volatile uint32_t ID_ISAR[5];   /* 0x060 */
    uint32_t          RESERVED0[5]; /* 0x074 - 0x084 */
    volatile uint32_t CPACR;        /* 0x088 */
} SCB_TypeDef;

#define PWR         ((PWR_TypeDef *)      PWR_BASE)
#define FLASH       ((FLASH_TypeDef *)    FLASH_REG_BASE)
#define RCC         ((RCC_TypeDef *)      RCC_BASE)
#define GPIOA       ((GPIO_TypeDef *)     GPIOA_BASE)
#define GPIOB       ((GPIO_TypeDef *)     GPIOB_BASE)
#define GPIOC       ((GPIO_TypeDef *)     GPIOC_BASE)
#define GPIOD       ((GPIO_TypeDef *)     GPIOD_BASE)
#define GPIOE       ((GPIO_TypeDef *)     GPIOE_BASE)
#define GPIOF       ((GPIO_TypeDef *)     GPIOF_BASE)
#define GPIOG       ((GPIO_TypeDef *)     GPIOG_BASE)
#define GPIOH       ((GPIO_TypeDef *)     GPIOH_BASE)
#define GPIOI       ((GPIO_TypeDef *)     GPIOI_BASE)
#define SysTick     ((SysTick_TypeDef *)  SYSTICK_BASE)
#define NVIC        ((NVIC_TypeDef *)     NVIC_BASE)
#define SCB         ((SCB_TypeDef *)      SCB_BASE)

/* ------------------------------- RCC bits -------------------------------- */
#define RCC_CR_HSION             (1U << 0)
#define RCC_CR_HSIRDY            (1U << 1)
#define RCC_CR_HSEON             (1U << 16)
#define RCC_CR_HSERDY            (1U << 17)
#define RCC_CR_HSEBYP            (1U << 18)
#define RCC_CR_CSSON             (1U << 19)
#define RCC_CR_PLLON             (1U << 24)
#define RCC_CR_PLLRDY            (1U << 25)

#define RCC_PLLCFGR_PLLM_Pos     0U
#define RCC_PLLCFGR_PLLN_Pos     6U
#define RCC_PLLCFGR_PLLP_Pos     16U
#define RCC_PLLCFGR_PLLQ_Pos     24U
#define RCC_PLLCFGR_PLLSRC_HSE   (1U << 22)

#define RCC_CFGR_SW_MASK         (3U << 0)
#define RCC_CFGR_SW_PLL          (2U << 0)
#define RCC_CFGR_SWS_MASK        (3U << 2)
#define RCC_CFGR_SWS_HSI         (0U << 2)
#define RCC_CFGR_SWS_HSE         (1U << 2)
#define RCC_CFGR_SWS_PLL         (2U << 2)
#define RCC_CFGR_HPRE_DIV1       (0U << 4)
#define RCC_CFGR_PPRE1_DIV4      (5U << 10)
#define RCC_CFGR_PPRE2_DIV2      (4U << 13)

#define RCC_AHB1ENR_GPIOAEN      (1U << 0)
#define RCC_AHB1ENR_GPIOBEN      (1U << 1)
#define RCC_AHB1ENR_GPIOCEN      (1U << 2)
#define RCC_AHB1ENR_GPIODEN      (1U << 3)
#define RCC_AHB1ENR_GPIOEEN      (1U << 4)
#define RCC_AHB1ENR_GPIOFEN      (1U << 5)
#define RCC_AHB1ENR_GPIOGEN      (1U << 6)
#define RCC_AHB1ENR_GPIOHEN      (1U << 7)
#define RCC_AHB1ENR_GPIOIEN      (1U << 8)
#define RCC_APB1ENR_PWREN        (1U << 28)

/* ------------------------------ FLASH bits ------------------------------ */
#define FLASH_ACR_LATENCY_5WS    (5U << 0)
#define FLASH_ACR_PRFTEN         (1U << 8)
#define FLASH_ACR_ICEN           (1U << 9)
#define FLASH_ACR_DCEN           (1U << 10)

/* ------------------------------- PWR bits ------------------------------- */
#define PWR_CR_VOS_MASK          (3U << 14)
#define PWR_CR_VOS_SCALE1        (3U << 14)   /* required for 168 MHz */

/* ------------------------------ SysTick bits ----------------------------- */
#define SYSTICK_CTRL_ENABLE      (1U << 0)
#define SYSTICK_CTRL_TICKINT     (1U << 1)
#define SYSTICK_CTRL_CLKSOURCE   (1U << 2)
#define SYSTICK_CTRL_COUNTFLAG   (1U << 16)

/* -------------------------------- SCB bits ------------------------------- */
#define SCB_CPACR_CP10_CP11_FULL (0xFU << 20)

/* --------------------------- IRQ numbers (RM0090) ------------------------ */
/* Matches the vector table in src/startup_stm32f407xx.c exactly. */
typedef enum {
    NonMaskableInt_IRQn   = -14,
    MemoryManagement_IRQn = -12,
    BusFault_IRQn         = -11,
    UsageFault_IRQn       = -10,
    SVCall_IRQn           = -5,
    DebugMonitor_IRQn     = -4,
    PendSV_IRQn           = -2,
    SysTick_IRQn          = -1,

    WWDG_IRQn = 0,
    PVD_IRQn,
    TAMP_STAMP_IRQn,
    RTC_WKUP_IRQn,
    FLASH_IRQn,
    RCC_IRQn,
    EXTI0_IRQn,
    EXTI1_IRQn,
    EXTI2_IRQn,
    EXTI3_IRQn,
    EXTI4_IRQn,
    DMA1_Stream0_IRQn,
    DMA1_Stream1_IRQn,
    DMA1_Stream2_IRQn,
    DMA1_Stream3_IRQn,
    DMA1_Stream4_IRQn,
    DMA1_Stream5_IRQn,
    DMA1_Stream6_IRQn,
    ADC_IRQn,
    CAN1_TX_IRQn,
    CAN1_RX0_IRQn,
    CAN1_RX1_IRQn,
    CAN1_SCE_IRQn,
    EXTI9_5_IRQn,
    TIM1_BRK_TIM9_IRQn,
    TIM1_UP_TIM10_IRQn,
    TIM1_TRG_COM_TIM11_IRQn,
    TIM1_CC_IRQn,
    TIM2_IRQn,
    TIM3_IRQn,
    TIM4_IRQn,
    I2C1_EV_IRQn,
    I2C1_ER_IRQn,
    I2C2_EV_IRQn,
    I2C2_ER_IRQn,
    SPI1_IRQn,
    SPI2_IRQn,
    USART1_IRQn,
    USART2_IRQn,
    USART3_IRQn,
    EXTI15_10_IRQn,
    RTC_Alarm_IRQn,
    OTG_FS_WKUP_IRQn,
    TIM8_BRK_TIM12_IRQn,
    TIM8_UP_TIM13_IRQn,
    TIM8_TRG_COM_TIM14_IRQn,
    TIM8_CC_IRQn,
    DMA1_Stream7_IRQn,
    FSMC_IRQn,
    SDIO_IRQn,
    TIM5_IRQn,
    SPI3_IRQn,
    UART4_IRQn,
    UART5_IRQn,
    TIM6_DAC_IRQn,
    TIM7_IRQn,
    DMA2_Stream0_IRQn,
    DMA2_Stream1_IRQn,
    DMA2_Stream2_IRQn,
    DMA2_Stream3_IRQn,
    DMA2_Stream4_IRQn,
    ETH_IRQn,
    ETH_WKUP_IRQn,
    CAN2_TX_IRQn,
    CAN2_RX0_IRQn,
    CAN2_RX1_IRQn,
    CAN2_SCE_IRQn,
    OTG_FS_IRQn,
    DMA2_Stream5_IRQn,
    DMA2_Stream6_IRQn,
    DMA2_Stream7_IRQn,
    USART6_IRQn,
    I2C3_EV_IRQn,
    I2C3_ER_IRQn,
    OTG_HS_EP1_OUT_IRQn,
    OTG_HS_EP1_IN_IRQn,
    OTG_HS_WKUP_IRQn,
    OTG_HS_IRQn,
    DCMI_IRQn,
    CRYP_IRQn,          /* reserved on F407: vector slot is hardwired to 0 */
    HASH_RNG_IRQn,
    FPU_IRQn
} IRQn_Type;

__attribute__((always_inline)) static inline void NVIC_EnableIRQ(IRQn_Type irq)
{
    if ((int32_t)irq >= 0) {
        NVIC->ISER[((uint32_t)irq >> 5)] = 1U << ((uint32_t)irq & 0x1FU);
    }
}

__attribute__((always_inline)) static inline void NVIC_DisableIRQ(IRQn_Type irq)
{
    if ((int32_t)irq >= 0) {
        NVIC->ICER[((uint32_t)irq >> 5)] = 1U << ((uint32_t)irq & 0x1FU);
    }
}

#ifdef __cplusplus
}
#endif

#endif /* STM32F407XX_H */
