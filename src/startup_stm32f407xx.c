/**
 * @file  startup_stm32f407xx.c
 * @brief C implementation of the STM32F407 reset path and interrupt vector table.
 *
 * Equivalent to ST's startup_stm32f407xx.s, but written in C so the whole boot
 * chain stays readable in one file. Vector order/names were checked against
 * STMicroelectronics/cmsis_device_f4 :: Source/Templates/gcc/startup_stm32f407xx.s
 *
 * Boot chain (see ../docs/boot-sequence.md):
 *   hardware loads MSP from 0x08000000 and jumps to Reset_Handler (0x08000004)
 *   -> .data copy / .bss zero -> SystemInit() -> __libc_init_array() -> main()
 */
#include <stdint.h>
#include "stm32f407xx.h"
#include "system_stm32f4xx.h"

/* Linker-script symbols */
extern uint32_t _estack;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

void Reset_Handler(void);
void Default_Handler(void);
void SystemInit(void);
void __libc_init_array(void);
int main(void);

typedef void (*vector_entry_t)(void);

/* Weak aliases: any strong definition in another translation unit wins. */
#define WEAK_IRQ(name) void name(void) __attribute__((weak, alias("Default_Handler")))

WEAK_IRQ(NMI_Handler);
WEAK_IRQ(HardFault_Handler);
WEAK_IRQ(MemManage_Handler);
WEAK_IRQ(BusFault_Handler);
WEAK_IRQ(UsageFault_Handler);
WEAK_IRQ(SVC_Handler);
WEAK_IRQ(DebugMon_Handler);
WEAK_IRQ(PendSV_Handler);
WEAK_IRQ(SysTick_Handler);

WEAK_IRQ(WWDG_IRQHandler);
WEAK_IRQ(PVD_IRQHandler);
WEAK_IRQ(TAMP_STAMP_IRQHandler);
WEAK_IRQ(RTC_WKUP_IRQHandler);
WEAK_IRQ(FLASH_IRQHandler);
WEAK_IRQ(RCC_IRQHandler);
WEAK_IRQ(EXTI0_IRQHandler);
WEAK_IRQ(EXTI1_IRQHandler);
WEAK_IRQ(EXTI2_IRQHandler);
WEAK_IRQ(EXTI3_IRQHandler);
WEAK_IRQ(EXTI4_IRQHandler);
WEAK_IRQ(DMA1_Stream0_IRQHandler);
WEAK_IRQ(DMA1_Stream1_IRQHandler);
WEAK_IRQ(DMA1_Stream2_IRQHandler);
WEAK_IRQ(DMA1_Stream3_IRQHandler);
WEAK_IRQ(DMA1_Stream4_IRQHandler);
WEAK_IRQ(DMA1_Stream5_IRQHandler);
WEAK_IRQ(DMA1_Stream6_IRQHandler);
WEAK_IRQ(ADC_IRQHandler);
WEAK_IRQ(CAN1_TX_IRQHandler);
WEAK_IRQ(CAN1_RX0_IRQHandler);
WEAK_IRQ(CAN1_RX1_IRQHandler);
WEAK_IRQ(CAN1_SCE_IRQHandler);
WEAK_IRQ(EXTI9_5_IRQHandler);
WEAK_IRQ(TIM1_BRK_TIM9_IRQHandler);
WEAK_IRQ(TIM1_UP_TIM10_IRQHandler);
WEAK_IRQ(TIM1_TRG_COM_TIM11_IRQHandler);
WEAK_IRQ(TIM1_CC_IRQHandler);
WEAK_IRQ(TIM2_IRQHandler);
WEAK_IRQ(TIM3_IRQHandler);
WEAK_IRQ(TIM4_IRQHandler);
WEAK_IRQ(I2C1_EV_IRQHandler);
WEAK_IRQ(I2C1_ER_IRQHandler);
WEAK_IRQ(I2C2_EV_IRQHandler);
WEAK_IRQ(I2C2_ER_IRQHandler);
WEAK_IRQ(SPI1_IRQHandler);
WEAK_IRQ(SPI2_IRQHandler);
WEAK_IRQ(USART1_IRQHandler);
WEAK_IRQ(USART2_IRQHandler);
WEAK_IRQ(USART3_IRQHandler);
WEAK_IRQ(EXTI15_10_IRQHandler);
WEAK_IRQ(RTC_Alarm_IRQHandler);
WEAK_IRQ(OTG_FS_WKUP_IRQHandler);
WEAK_IRQ(TIM8_BRK_TIM12_IRQHandler);
WEAK_IRQ(TIM8_UP_TIM13_IRQHandler);
WEAK_IRQ(TIM8_TRG_COM_TIM14_IRQHandler);
WEAK_IRQ(TIM8_CC_IRQHandler);
WEAK_IRQ(DMA1_Stream7_IRQHandler);
WEAK_IRQ(FSMC_IRQHandler);
WEAK_IRQ(SDIO_IRQHandler);
WEAK_IRQ(TIM5_IRQHandler);
WEAK_IRQ(SPI3_IRQHandler);
WEAK_IRQ(UART4_IRQHandler);
WEAK_IRQ(UART5_IRQHandler);
WEAK_IRQ(TIM6_DAC_IRQHandler);
WEAK_IRQ(TIM7_IRQHandler);
WEAK_IRQ(DMA2_Stream0_IRQHandler);
WEAK_IRQ(DMA2_Stream1_IRQHandler);
WEAK_IRQ(DMA2_Stream2_IRQHandler);
WEAK_IRQ(DMA2_Stream3_IRQHandler);
WEAK_IRQ(DMA2_Stream4_IRQHandler);
WEAK_IRQ(ETH_IRQHandler);
WEAK_IRQ(ETH_WKUP_IRQHandler);
WEAK_IRQ(CAN2_TX_IRQHandler);
WEAK_IRQ(CAN2_RX0_IRQHandler);
WEAK_IRQ(CAN2_RX1_IRQHandler);
WEAK_IRQ(CAN2_SCE_IRQHandler);
WEAK_IRQ(OTG_FS_IRQHandler);
WEAK_IRQ(DMA2_Stream5_IRQHandler);
WEAK_IRQ(DMA2_Stream6_IRQHandler);
WEAK_IRQ(DMA2_Stream7_IRQHandler);
WEAK_IRQ(USART6_IRQHandler);
WEAK_IRQ(I2C3_EV_IRQHandler);
WEAK_IRQ(I2C3_ER_IRQHandler);
WEAK_IRQ(OTG_HS_EP1_OUT_IRQHandler);
WEAK_IRQ(OTG_HS_EP1_IN_IRQHandler);
WEAK_IRQ(OTG_HS_WKUP_IRQHandler);
WEAK_IRQ(OTG_HS_IRQHandler);
WEAK_IRQ(DCMI_IRQHandler);
/* slot 79 (CRYP) is reserved on STM32F407 -> hardwired to 0 in the table */
WEAK_IRQ(HASH_RNG_IRQHandler);
WEAK_IRQ(FPU_IRQHandler);

/**
 * Vector table at flash offset 0. The first word is the initial MSP, the second
 * is the reset vector - both consumed automatically by the Cortex-M4 core.
 */
const vector_entry_t g_pfnVectors[]
    __attribute__((used, section(".isr_vector"))) = {
        (vector_entry_t)&_estack,
        Reset_Handler,
        NMI_Handler,
        HardFault_Handler,
        MemManage_Handler,
        BusFault_Handler,
        UsageFault_Handler,
        0, 0, 0, 0,             /* reserved */
        SVC_Handler,
        DebugMon_Handler,
        0,                       /* reserved */
        PendSV_Handler,
        SysTick_Handler,

        /* STM32F407 external interrupts - 82 slots, RM0090 vector order */
        WWDG_IRQHandler,
        PVD_IRQHandler,
        TAMP_STAMP_IRQHandler,
        RTC_WKUP_IRQHandler,
        FLASH_IRQHandler,
        RCC_IRQHandler,
        EXTI0_IRQHandler,
        EXTI1_IRQHandler,
        EXTI2_IRQHandler,
        EXTI3_IRQHandler,
        EXTI4_IRQHandler,
        DMA1_Stream0_IRQHandler,
        DMA1_Stream1_IRQHandler,
        DMA1_Stream2_IRQHandler,
        DMA1_Stream3_IRQHandler,
        DMA1_Stream4_IRQHandler,
        DMA1_Stream5_IRQHandler,
        DMA1_Stream6_IRQHandler,
        ADC_IRQHandler,
        CAN1_TX_IRQHandler,
        CAN1_RX0_IRQHandler,
        CAN1_RX1_IRQHandler,
        CAN1_SCE_IRQHandler,
        EXTI9_5_IRQHandler,
        TIM1_BRK_TIM9_IRQHandler,
        TIM1_UP_TIM10_IRQHandler,
        TIM1_TRG_COM_TIM11_IRQHandler,
        TIM1_CC_IRQHandler,
        TIM2_IRQHandler,
        TIM3_IRQHandler,
        TIM4_IRQHandler,
        I2C1_EV_IRQHandler,
        I2C1_ER_IRQHandler,
        I2C2_EV_IRQHandler,
        I2C2_ER_IRQHandler,
        SPI1_IRQHandler,
        SPI2_IRQHandler,
        USART1_IRQHandler,
        USART2_IRQHandler,
        USART3_IRQHandler,
        EXTI15_10_IRQHandler,
        RTC_Alarm_IRQHandler,
        OTG_FS_WKUP_IRQHandler,
        TIM8_BRK_TIM12_IRQHandler,
        TIM8_UP_TIM13_IRQHandler,
        TIM8_TRG_COM_TIM14_IRQHandler,
        TIM8_CC_IRQHandler,
        DMA1_Stream7_IRQHandler,
        FSMC_IRQHandler,
        SDIO_IRQHandler,
        TIM5_IRQHandler,
        SPI3_IRQHandler,
        UART4_IRQHandler,
        UART5_IRQHandler,
        TIM6_DAC_IRQHandler,
        TIM7_IRQHandler,
        DMA2_Stream0_IRQHandler,
        DMA2_Stream1_IRQHandler,
        DMA2_Stream2_IRQHandler,
        DMA2_Stream3_IRQHandler,
        DMA2_Stream4_IRQHandler,
        ETH_IRQHandler,
        ETH_WKUP_IRQHandler,
        CAN2_TX_IRQHandler,
        CAN2_RX0_IRQHandler,
        CAN2_RX1_IRQHandler,
        CAN2_SCE_IRQHandler,
        OTG_FS_IRQHandler,
        DMA2_Stream5_IRQHandler,
        DMA2_Stream6_IRQHandler,
        DMA2_Stream7_IRQHandler,
        USART6_IRQHandler,
        I2C3_EV_IRQHandler,
        I2C3_ER_IRQHandler,
        OTG_HS_EP1_OUT_IRQHandler,
        OTG_HS_EP1_IN_IRQHandler,
        OTG_HS_WKUP_IRQHandler,
        OTG_HS_IRQHandler,
        DCMI_IRQHandler,
        0,                       /* CRYP: not present on STM32F407 */
        HASH_RNG_IRQHandler,
        FPU_IRQHandler,
};

/**
 * newlib hooks required because we link with -nostartfiles.
 *
 * __libc_init_array() calls _init() unconditionally; normally crti.o/crtn.o
 * supply _init/_fini, but -nostartfiles removes those objects (we provide our
 * own reset entry point instead). Weak no-ops let the application override
 * them with a strong definition if it needs real C++ runtime setup.
 */
void _init(void) __attribute__((weak));
void _fini(void) __attribute__((weak));
void _init(void) { }
void _fini(void) { }

/** Trap for any unhandled exception - keep the state visible to a debugger.
 *  A plain NOP loop is used instead of BKPT: without a debugger attached,
 *  BKPT escalates to HardFault and would recurse into this same handler.
 *  Put a breakpoint here during bring-up to catch unexpected interrupts. */
void Default_Handler(void)
{
    for (;;) {
        __asm__ volatile("nop");
    }
}

/** Reset entry point. Runs with the stack pointer already loaded by hardware. */
__attribute__((noreturn)) void Reset_Handler(void)
{
    uint32_t *src;
    uint32_t *dst;

    /* Explicitly re-load MSP so the same image also works when entered from a
       debugger or jumped to by a custom bootloader (ST's .s startup does this). */
    __asm__ volatile("msr msp, %0" : : "r"(&_estack) : "memory");

    /* Enable the FPU before any compiler-generated floating point instruction. */
    SCB->CPACR |= SCB_CPACR_CP10_CP11_FULL;
    __asm__ volatile("dsb" ::: "memory");
    __asm__ volatile("isb" ::: "memory");

    /* .data: copy initializers from flash (LMA) into SRAM (VMA) */
    src = &_sidata;
    for (dst = &_sdata; dst < &_edata;) {
        *dst++ = *src++;
    }

    /* .bss: zero */
    for (dst = &_sbss; dst < &_ebss;) {
        *dst++ = 0U;
    }

    SystemInit();           /* HSE -> PLL -> 168 MHz, VTOR, flash wait states */
    __libc_init_array();    /* C++ static constructors (no-op in a pure C build) */
    (void)main();

    for (;;) {
        /* main() must never return; trap if it does. */
    }
}
