# 本工程的真实启动链路

把《STM32F407 vs ESP32 启动流程》信息图左半边的 5 个步骤，逐一对到这个工程里的具体文件和符号。所有地址均为默认配置（BOOT0 = 0、无 bootloader 偏移）。

| # | 信息图里的步骤 | 本工程对应位置 | 关键值 |
| --- | --- | --- | --- |
| 1 | 复位释放 | 硬件；`linker/*.ld` 的 `_estack` | POR / NRST / WDT / 软件复位；复位后默认 HSI 16 MHz |
| 2 | 采样 BOOT[1:0]，地址别名 | 硬件 + 跳线；SYSCLK 第 4 个上升沿锁存 | BOOT0=0 → 主 Flash `0x0800 0000` 别名到 `0x0000 0000`；BOOT0=1/BOOT1=0 → 系统存储器 `0x1FFF 0000`（ST ROM ISP）；BOOT0=1/BOOT1=1 → SRAM `0x2000 0000` |
| 3 | 硬件装载 MSP 与复位向量 | `src/startup_stm32f407xx.c` 的 `g_pfnVectors[]`，由 `.isr_vector` 段放到 flash 偏移 0 | `0x0000 0000` = `_estack` = `0x2002 0000`；`0x0000 0004` = `Reset_Handler` |
| 4 | Reset_Handler 汇编启动文件 | 同一个文件里的 **C 版** `Reset_Handler()` | 重设 MSP → 使能 FPU → 复制 `.data` → 清零 `.bss` → `SystemInit()` → `__libc_init_array()` → `main()` |
| 5 | `__main → main()` | `src/main.c` 的 `main()` | `led_init()` → `systick_init_1ms()` → 无限循环闪烁 PD12 |

## 各文件的职责

- **`linker/STM32F407VGTx_FLASH.ld`**：定义 FLASH 1 MB @ `0x08000000`、RAM 128 KB @ `0x20000000`、CCMRAM 64 KB @ `0x10000000`；`_estack` 指向 RAM 顶端；`_sidata` 是 `.data` 在 flash 里的加载地址（LMA），`_sdata`/`_edata`/`_sbss`/`_ebss` 是 SRAM 里的运行地址。链接脚本还用 `PROVIDE_HIDDEN` 导出了 `__init_array_start/end` 等符号，供 `__libc_init_array()` 使用。
- **`src/startup_stm32f407xx.c`**：`g_pfnVectors[]` 共 **98 项** = 16 个 Cortex-M4 核异常 + 82 个外部中断，顺序与 ST 官方 `cmsis_device_f4/Source/Templates/gcc/startup_stm32f407xx.s` 一致（其中第 80 项 CRYP 在 F407 上不存在，硬编码为 `0`）。所有 handler 都是 `__attribute__((weak, alias("Default_Handler")))`，任何同名的强定义会自动覆盖——例如 `main.c` 里的 `SysTick_Handler`。
- **`src/system_stm32f4xx.c`**：`SystemInit()` 完成 `SystemCoreClockUpdate` 之外的全部时钟工作：CP10/CP11 全访问（FPU）→ HSI 兜底 → HSE 起振（带超时，失败则留在 16 MHz 而不是死等）→ PWR VOS Scale 1 → AHB/1、APB1/4、APB2/2 → PLL `(HSE/8)×336/2 = 168 MHz`（PLLQ=7 给 USB 48 MHz）→ Flash 5 等待周期 + 预取/I-Cache/D-Cache → 切 SW 到 PLL → `SCB->VTOR = 0x08000000`。
- **`inc/stm32f407xx.h`**：精简寄存器定义与 IRQn 枚举；不是完整 CMSIS，只覆盖本模板用到的外设。

## 两个值得注意的实现细节

1. **顺序与 ST 汇编模板略有不同**：ST 的 `startup_stm32f407xx.s` 先 `bl SystemInit` 再复制 `.data`/清零 `.bss`；本模板先建立 C 运行环境再调用 `SystemInit()`。因为 `SystemInit()` 会读写 `SystemCoreClock` 这个带初值的全局变量，先做 `.data` 复制更稳妥。两者在纯寄存器操作下都可用。
2. **显式重设 MSP**：硬件已经从 `0x0000 0000` 装载了 MSP，但 `Reset_Handler()` 里还是 `msr msp, _estack` 一次，这样同一份镜像被调试器直接加载、或被自定义 IAP bootloader 跳转进来时也能正常工作。

## 如果加自定义 bootloader

App 需要改动三处（这是信息图里「IAP 是可选的」那句话的落地）：

1. `linker/*.ld`：FLASH 的 `ORIGIN` 改为 `0x08008000`（举例，偏移 32 KB），`LENGTH` 相应减少；
2. `SystemInit()`：`SCB->VTOR = 0x08008000`（VTOR 必须按向量表大小向上取 2 的幂对齐，98×4=392 B → 512 B 对齐）；
3. Bootloader 跳转前：关中断、停 SysTick、设置 `SCB->VTOR`、从 App 向量表首字取 MSP、次字取复位向量再跳转。

对比 ESP32：`Mask ROM → flash 0x1000 二级 bootloader → 分区表 0x8000 → app` 是芯片强制的三级链，无法像这里一样「跳过」；STM32F407 的整条链完全由用户掌握，这也是本工程能只用 3 个 `.c` 文件跑起来的原因。
