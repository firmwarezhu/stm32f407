# STM32F407 裸机工程模板（Cursor / VS Code）

一个**不依赖 CubeMX、HAL 或任何下载包**的 STM32F407 最小工程：自带向量表、`Reset_Handler`、链接脚本、168 MHz 时钟树、1 ms SysTick 时基和 LED 闪烁示例。装好 `arm-none-eabi-gcc` 就能编译，用 Cursor 一键构建 / 烧录 / 调试。

这条启动链正好对应上一张《STM32F407 vs ESP32 启动流程》信息图的左半边，详见 [docs/boot-sequence.md](docs/boot-sequence.md)。

---

## 1. 默认硬件假设

| 项目 | 默认值 | 修改位置 |
| --- | --- | --- |
| MCU | STM32F407VGT6（1 MB Flash / 128 KB SRAM / 64 KB CCM） | `linker/STM32F407VGTx_FLASH.ld` |
| 启动模式 | BOOT0 = 0，主 Flash `0x0800 0000` 别名到 `0x0000 0000` | 硬件跳线 |
| HSE 晶振 | 8 MHz（STM32F4-Discovery） | `-DHSE_VALUE=...` 或 `inc/board.h` |
| SYSCLK | HSE / 8 × 336 / 2 = **168 MHz**；APB1 42 MHz、APB2 84 MHz | `src/system_stm32f4xx.c` |
| 用户 LED | PD12（Discovery 绿色 LED，高电平点亮） | `inc/board.h` |
| 调试接口 | SWD，**J-Link（默认）**；ST-Link 为备选 | `.vscode/launch.json`、`openocd.cfg` |
| J-Link 目标型号 | `STM32F407VG` | `tools/flash.jlink`、`.vscode/launch.json` |
| 链接脚本 Flash | 1024 KB（VGT6） | `linker/STM32F407VGTx_FLASH.ld` |

换成自己的板子，通常只改 `inc/board.h` 的 `BOARD_LED_PORT` / `BOARD_LED_PIN` / `BOARD_LED_RCC_BIT` 和 `HSE_VALUE`。PLL 的 `PLL_M` 由 `HSE_VALUE / 1 MHz` 自动推导，8 / 12 / 16 / 25 MHz 晶振都能得到 168 MHz。芯片若是 VET6 / ZGT6，只要固件远小于 512 KB，现有链接脚本照样能跑；但 J-Link 的 `device` 名要改成对应型号。

### 硬件清单（J-Link + F407 EVB）

| # | 硬件 | 说明 |
| --- | --- | --- |
| 1 | STM32F407 EVB | 需确认：MCU 具体型号、HSE 晶振频率（常见 8 MHz / 25 MHz）、用户 LED 所在 GPIO |
| 2 | J-Link 调试器 | 支持 SWD 的 V9 / V10 / V11 / EDU / PLUS 均可；**兼容克隆版可能被较新 SEGGER 驱动拒绝**，见第 8 节 |
| 3 | J-Link 的 USB 线 | 连接 PC，同时给 J-Link 供电 |
| 4 | SWD 连接线 | 20-pin/10-pin 排线，或 4~5 根杜邦线，见下表 |
| 5 | EVB 独立供电 | USB 线或外部 5V/3.3V。**J-Link 的 VTref 是电压检测脚，不给目标板供电** |
| 6 | BOOT0 跳线 | 置 0（从主 Flash `0x0800 0000` 启动）；BOOT1/PB2 也应为 0 |

SWD 接线（J-Link 标准 20-pin ARM 接头）：

| J-Link 20-pin | 信号 | F407 引脚 | 作用 |
| --- | --- | --- | --- |
| Pin 1 | VTref | 3.3V | 电平参考，**只检测不供电** |
| Pin 7 | SWDIO / TMS | PA13 | 双向数据 |
| Pin 9 | SWCLK / TCK | PA14 | 时钟 |
| Pin 15 | nRESET | NRST | 建议接，支持 connect-under-reset |
| Pin 4/6/8/... | GND | GND | 至少一根，必须共地 |

若 EVB 是 10-pin Cortex Debug 接头（1.27 mm）：Pin1 VTref、Pin2 SWDIO、Pin3 GND、Pin4 SWCLK、Pin5 GND、Pin10 nRESET。

> 线长尽量 < 15 cm；初次连接把 SWD 速率降到 1000 kHz（`tools/flash.jlink` 的 `speed` 与 `openocd*.cfg` 的 `adapter speed`），稳定后再往上调。SWDIO/SWCLK 接反不会烧板，但会表现为「探针正常、目标连不上」。

---

## 2. 目录结构

```
stm32f407-baremetal/
├── .vscode/                  Cursor / VS Code 配置
│   ├── c_cpp_properties.json  IntelliSense（ARM GCC + compile_commands）
│   ├── settings.json
│   ├── tasks.json             构建 / 烧录 / size / clean
│   ├── launch.json            Cortex-Debug（OpenOCD + ST-Link）
│   └── extensions.json        推荐扩展
├── cmake/arm-none-eabi-gcc.cmake   交叉编译工具链文件
├── CMakeLists.txt / CMakePresets.json
├── linker/STM32F407VGTx_FLASH.ld   内存布局、.data/.bss 符号
├── inc/
│   ├── stm32f407xx.h          精简寄存器定义（RCC/GPIO/FLASH/PWR/SysTick/SCB/NVIC）
│   ├── board.h                板级引脚与晶振配置
│   └── system_stm32f4xx.h     SystemCoreClock / SystemInit API
├── src/
│   ├── startup_stm32f407xx.c  向量表（16 核异常 + 82 外部中断）+ Reset_Handler
│   ├── system_stm32f4xx.c     时钟树：HSE -> PLL 168 MHz、VTOR、Flash 5 WS
│   └── main.c                 SysTick 1 ms + PD12 闪烁
├── tools/
│   ├── build.py               零依赖构建脚本（只需 Python + ARM GCC）
│   ├── flash.jlink            J-Link Commander 烧录脚本
│   ├── setup-toolchain-path.ps1  自动发现工具链并写入用户 PATH
│   └── README-toolchain.md    本机安装记录 + 硬件/驱动说明
├── openocd.cfg                ST-Link 接口配置
├── openocd-jlink.cfg          J-Link 走 OpenOCD 的备用配置
└── docs/boot-sequence.md
```

---

## 3. 安装工具链

本机已装好（2026-11-14 实际验证过），全部通过 winget 安装：

| 工具 | winget 包 | 版本 | 安装位置 |
| --- | --- | --- | --- |
| Arm GNU Toolchain | `Arm.ArmGnuToolchain` | 12.2.MPACBTI-Rel1 (GCC 12.2.1) | `C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\12.2 mpacbti-rel1\bin` |
| CMake | `Kitware.CMake` | 4.4.4 | 由安装器写入 PATH |
| Ninja | `Ninja-build.Ninja` | 1.13.2 | 由安装器写入 PATH |
| OpenOCD | `xpack-dev-tools.openocd-xpack` | 0.12.0-7 | 见 `tools/README-toolchain.md` |
| J-Link 软件包 | `NordicSemiconductor.JLink` | V8.18 | `C:\Program Files\SEGGER\JLink_V818`（含 `JLink.exe` / `JLinkGDBServerCL.exe` / USB 驱动） |

一键复现：

```bash
winget install --id Arm.ArmGnuToolchain -e
winget install --id Kitware.CMake -e
winget install --id Ninja-build.Ninja -e
winget install --id xpack-dev-tools.openocd-xpack -e
winget install --id NordicSemiconductor.JLink -e
powershell -ExecutionPolicy Bypass -File tools\setup-toolchain-path.ps1
```

J-Link 用 SEGGER 原生工具链，**不需要 Zadig 替换驱动**；装完把 J-Link 插上 PC，等设备管理器识别即可。

> **注意**：Arm 的 NSIS 安装器用 winget 静默安装时**不会勾选 Add to PATH**，所以 `arm-none-eabi-gcc` 默认不在 PATH 里。安装细节、PATH 处理办法和 `ARM_TOOLCHAIN_BIN` 用法见 [tools/README-toolchain.md](tools/README-toolchain.md)。`tools/build.py` 也会自动在常见安装目录里探测工具链，因此即使 PATH 未刷新，Python 快速构建路径也能跑；但 **CMake、Cursor 的 IntelliSense 和 F5 调试仍依赖 PATH**，改完 PATH 后需重启 Cursor。

改完 PATH 后重开终端验证：

```bash
arm-none-eabi-gcc --version
cmake --version
ninja --version
openocd --version
```

- 烧录调试还需要 ST-Link；也可改用 `st-flash`（stlink 工具）或 STM32CubeProgrammer 代替 OpenOCD。
- Cursor 扩展：打开工程时会提示安装 `Cortex-Debug`、`C/C++`、`CMake Tools`（已在 `extensions.json` 推荐）。

---

## 4. 用 Cursor 打开

```bash
cd C:/Users/firmw/WorkBuddy/2026-11-13-21-45-14
cursor stm32f407-baremetal
```

或在 Cursor 里 `File > Open Folder...` 选择 `stm32f407-baremetal` 目录（**必须打开这个子目录**，否则 `${workspaceFolder}` 会指错位置）。

---

## 5. 构建

### 方式 A：Python 快速路径（推荐，不需要 CMake / Ninja / Make）

```bash
python tools/build.py                 # Debug: -Og -g3
python tools/build.py --release       # Release: -Os
python tools/build.py --hse 25000000  # 25 MHz 晶振
python tools/build.py --clean
```

产物：

```
build/direct/stm32f407-blink.elf   (调试用，带符号)
build/direct/stm32f407-blink.hex
build/direct/stm32f407-blink.bin
build/direct/stm32f407-blink.map
compile_commands.json              (供 IntelliSense / clangd)
```

在 Cursor 里直接按 **Ctrl+Shift+B** 即可（默认构建任务已指向该脚本）。

> 已用 Arm GNU Toolchain 12.2.1 实测编译通过：`.isr_vector` 392 B（98 项）、`.text` 780 B、`.data` 4 B，初始 MSP `0x20020000`、复位向量 `0x0800018D`（指向 `Reset_Handler`，Thumb 位置 1），ELF 标记为 hard-float ABI。

### 方式 B：CMake Presets（团队协作 / 多配置）

```bash
cmake --preset debug
cmake --build --preset debug
cmake --preset release && cmake --build --preset release
```

产物在 `build/Debug/` 或 `build/Release/`。CMake 构建会自动把 `compile_commands.json` 复制到工程根目录供 IntelliSense 使用。

> 两条路径生成的 ELF 等价；`.vscode/launch.json` 默认指向方式 A 的 `build/direct/*.elf`。若只用 CMake，把 `executable` 改成 `build/Debug/stm32f407-blink.elf`。

---

## 6. 烧录

**首选：J-Link 原生烧录**

```bash
JLink -device STM32F407VG -if SWD -speed 4000 -CommanderScript tools/flash.jlink
```

脚本 `tools/flash.jlink` 依次执行 `connect → halt → loadfile .hex → reset → go → exit`。芯片型号不是 VGT6 时，把 `-device` 和脚本里的 `device` 一起改成 `STM32F407VE` / `STM32F407ZG` 等。

其他路径：

```bash
# OpenOCD + ST-Link（烧 ELF，自动校验并复位）
openocd -f openocd.cfg -c "program build/direct/stm32f407-blink.elf verify reset exit"

# OpenOCD 驱动 J-Link（需 WinUSB，见 openocd-jlink.cfg 注释）
openocd -f openocd-jlink.cfg -c "program build/direct/stm32f407-blink.elf verify reset exit"

# st-flash（winget 无此包，需自行安装）
st-flash write build/direct/stm32f407-blink.bin 0x08000000
```

Cursor 里用 `Ctrl+Shift+P > Tasks: Run Task`，选 **`Flash: J-Link (HEX)`**（默认）或 `Flash: OpenOCD (ELF)`。

---

## 7. 调试

装好 `Cortex-Debug` 扩展与 J-Link 软件包、接好 SWD、给 EVB 独立供电并置 BOOT0=0，按 **F5** 选择 **`Debug STM32F407 (J-Link / SWD)`**（列表第一项）：自动构建 → 启动 `JLinkGDBServerCL` → 烧录 → 停在 `main()`。支持断点、寄存器视图、外设寄存器（SVD 可自行添加）、Live Watch。

`launch.json` 一共 3 个配置：J-Link 原生（默认）、ST-Link + OpenOCD、J-Link + OpenOCD（备用）。若 `JLinkGDBServerCL.exe` 找不到，在对应配置里加 `"serverpath": "C:/Program Files/SEGGER/JLink_V818/JLinkGDBServerCL.exe"`；GDB 找不到则把 `gdbPath` 改成 `arm-none-eabi-gdb` 的绝对路径。

> 改过 PATH 之后必须**重启 Cursor**，否则 Cortex-Debug 找不到 `JLinkGDBServerCL.exe` 和 `arm-none-eabi-gdb`。

---

## 8. 常见坑

| 现象 | 原因 / 处理 |
| --- | --- |
| 上电后 LED 闪得很慢 | HSE 没起振，代码已自动**回退 HSI 16 MHz**（`SystemCoreClock` 会变成 16000000），不是死机。检查晶振/负载电容，或改用 `--hse` 指定正确频率 |
| 链接报 `undefined reference to _init`（来自 `__libc_init_array`） | `-nostartfiles` 去掉了 crti.o/crtn.o，而 newlib 的 `__libc_init_array()` 会调用 `_init()`。`startup_stm32f407xx.c` 里已提供 weak 空实现 `_init` / `_fini`；不要删，也不要加 `-nostdlib`。保留 `--specs=nano.specs --specs=nosys.specs` |
| 链接报 `undefined reference to __libc_init_array` | 链接参数漏了标准库，或自己删掉了 `Reset_Handler` 里的调用；保留 `--specs=nano.specs --specs=nosys.specs` |
| 一进中断就 HardFault | 向量表名字写错或 VTOR 未指向 `0x08000000`；`SystemInit()` 里已设置 `SCB->VTOR = FLASH_BASE` |
| 自己写的 `USART1_IRQHandler` 不触发 | 函数名必须与 `startup_stm32f407xx.c` 向量表完全一致；强定义会覆盖 startup 里的 weak 别名，还要记得 `NVIC_EnableIRQ(USART1_IRQn)` |
| 加了自定义 bootloader 后中断全跑飞 | App 链接地址偏移后，必须同步修改 `.ld` 的 FLASH ORIGIN 并重设 `SCB->VTOR`（VTOR 需按向量表大小向上取 2 的幂对齐，F407 为 512 字节） |
| CCM RAM 里的数据 DMA 搬不动 | CCM (`0x10000000`) 只有 CPU 能访问，DMA 缓冲区必须放普通 SRAM (`0x20000000`) |
| 用 `printf` 没有输出 | 本模板未实现 `_write`；需自己实现 UART 版 `_write`，或先不用标准 IO |
| J-Link 连不上目标 / `Cannot connect to target` | ① 目标板没独立供电（VTref 不供电）；② 没共地；③ SWDIO/SWCLK 接反；④ BOOT0 被拉到 1；⑤ 先用 `speed 1000` 降速再试；⑥ 接上 nRESET 后用 connect-under-reset |
| SEGGER 软件报探针非正版 / 克隆 J-Link 被拒绝 | 较新 J-Link 驱动会拒绝兼容克隆。可换用正版 J-Link、装旧版软件包，或改用 `openocd-jlink.cfg` / CMSIS-DAP 路径 |
| 烧录成功但 LED 不闪 | 大概率是 EVB 的 LED 不在 PD12：改 `inc/board.h` 的 `BOARD_LED_PORT` / `BOARD_LED_PIN` / `BOARD_LED_RCC_BIT` / `BOARD_LED_ACTIVE_HIGH`；部分板子 LED 低电平点亮 |
| LED 闪得很慢或时序不对 | HSE 频率与默认 8 MHz 不符：用 `python tools/build.py --hse 25000000` 指定，或改 `inc/board.h` 的 `HSE_VALUE`。`SystemInit()` 在 HSE 不起振时会回退 HSI 16 MHz，所以不会死机，只是变慢 |
| `Flash download failed` / Flash 算法不匹配 | `tools/flash.jlink` 与 `launch.json` 里的 `device` 是 `STM32F407VG`；VET6 / ZGT6 请改成对应型号 |

---

## 9. 升级到 HAL / CMSIS / 其他生态

本工程刻意只用精简寄存器头（约 300 行），方便把启动流程看清楚。项目变大时：

- **CubeMX + HAL**：用 CubeMX 生成 `.ioc` 与 `Drivers/`（STM32CubeF4），保留本工程的 `CMakeLists.txt` / `tools/build.py` 构建框架，把 `inc/stm32f407xx.h` 换成官方 CMSIS 头，删掉自写的 `startup_*.c` 和 `system_*.c`，改用 Cube 提供的版本；`main.c` 换成 `HAL_Init()` + `SystemClock_Config()` + `MX_GPIO_Init()`。
- **PlatformIO**：新建 `platformio.ini`，`platform = ststm32`、`board = disco_f407vg`、`framework = stm32cube`，把 `src/` 内容移入 `src/` 即可。
- **CMSIS 设备包**：`STMicroelectronics/cmsis_device_f4` 提供官方 `startup_stm32f407xx.s` 与完整 `stm32f407xx.h`；本工程向量表顺序就是照它核对的。

---

## 10. 参考

- ST RM0090 — STM32F405/407 参考手册（Boot configuration、RCC、FLASH、PWR）
- ST AN2606 — STM32 系统存储器 bootloader
- `STMicroelectronics/cmsis_device_f4` — `Source/Templates/gcc/startup_stm32f407xx.s`（向量表）
- Arm GNU Toolchain / newlib（`--specs=nano.specs`、`--specs=nosys.specs`）
