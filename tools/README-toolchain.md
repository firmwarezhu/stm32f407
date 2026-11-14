# 本机工具链安装记录（2026-11-14）

全部通过 winget 静默安装，`tools/setup-toolchain-path.ps1` 已把可执行文件目录写入**用户级 PATH**。

## 已装组件

| 工具 | winget ID | 版本 | 可执行文件目录 |
| --- | --- | --- | --- |
| Arm GNU Toolchain | `Arm.ArmGnuToolchain` | 12.2.MPACBTI-Rel1 / GCC 12.2.1 | `C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\12.2 mpacbti-rel1\bin` |
| CMake | `Kitware.CMake` | 4.4.4 | `%LOCALAPPDATA%\Microsoft\WinGet\Packages\Kitware.CMake_*\cmake-4.4.4-windows-x86_64\bin` |
| Ninja | `Ninja-build.Ninja` | 1.13.2 | `%LOCALAPPDATA%\Microsoft\WinGet\Packages\Ninja-build.Ninja_*` |
| OpenOCD (xPack) | `xpack-dev-tools.openocd-xpack` | 0.12.0+dev | `%LOCALAPPDATA%\Microsoft\WinGet\Packages\xpack-dev-tools.openocd-xpack_*\xpack-openocd-0.12.0-7\bin` |
| J-Link 软件包 | `NordicSemiconductor.JLink` | V8.18 | `C:\Program Files\SEGGER\JLink_V818`（`JLink.exe` / `JLinkGDBServerCL.exe`） |

winget 另外在 `%LOCALAPPDATA%\Microsoft\WinGet\Links` 里放了 `cmake.exe` / `ninja.exe` 软链，该目录也已加入用户 PATH。

## 两个坑

1. **Arm 的 NSIS 安装器在 winget 静默模式下不会勾选 "Add to PATH"** —— 所以 `arm-none-eabi-gcc` 装完默认找不到。这就是 `setup-toolchain-path.ps1` 存在的原因。
2. **CMake / Ninja / OpenOCD 是便携包**，安装目录名带版本号和随机后缀，`winget upgrade --all` 之后路径会变。升级后重跑一次脚本即可：

```powershell
powershell -ExecutionPolicy Bypass -File tools\setup-toolchain-path.ps1
```

脚本是幂等的（重复运行不会往 PATH 里塞重复项），运行时也会顺带更新当前窗口的 `$env:Path`。

## 生效方式

PATH 改动只对**之后新开的进程**生效。已经开着的 Cursor / 终端不会自动刷新，必须重启 Cursor（不是只重载窗口）。

新开终端验证：

```bash
arm-none-eabi-gcc --version
cmake --version
ninja --version
openocd --version
JLink -?          # 应显示 SEGGER J-Link Commander V8.18
where.exe JLinkGDBServerCL.exe
```

## 不想动系统 PATH

`tools/build.py` 会自动在常见安装目录里探测 ARM 工具链，所以 Python 快速构建路径不依赖 PATH。也可以显式指定：

```bash
# Windows PowerShell
$env:ARM_TOOLCHAIN_BIN = "C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\12.2 mpacbti-rel1\bin"
python tools/build.py

# bash
export ARM_TOOLCHAIN_BIN="/c/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/12.2 mpacbti-rel1/bin"
python tools/build.py
```

注意 **CMake Presets、Cursor 的 C/C++ IntelliSense 和 Cortex-Debug (F5) 仍然依赖 PATH**，因为它们直接按名字调用 `arm-none-eabi-gcc` / `openocd`。

## 实测结果

| 项目 | 结果 |
| --- | --- |
| `python tools/build.py` (Debug) | 通过，`text 1204 / data 4 / bss 1540` |
| `cmake --preset debug && cmake --build --preset debug` | 通过，产物一致 |
| `cmake --preset release && cmake --build --preset release` | 通过，`-Os` 后 `text 1196` |
| ELF 校验 | ELF32 / ARM / hard-float ABI；`.isr_vector` 392 B；初始 MSP `0x20020000`；复位向量 `0x0800018D` |
| `openocd -f openocd.cfg -c "shutdown"` | 配置解析通过（不接硬件） |
| `openocd -f openocd-jlink.cfg -c "shutdown"` | 配置解析通过（不接硬件） |
| 实际烧录 / 上板调试 | **未验证**，需要接上 J-Link + F407 EVB |

## 调试器：本机用 J-Link

用户手上有 **J-Link + STM32F407 EVB**，所以工程默认路径已切到 J-Link 原生工具（`launch.json` 第一项 `servertype: jlink`，`Flash: J-Link (HEX)` 任务）。好处是不用给 ST-Link 换 WinUSB/Zadig 驱动。

- `tools/flash.jlink` 是 J-Link Commander 脚本，`device STM32F407VG`；芯片型号不同要同步改 `launch.json` 的 `device`。
- `openocd-jlink.cfg` 是「OpenOCD 驱动 J-Link」的备用配置，需要探针绑到 WinUSB，SEGGER 原生驱动下可能打不开，优先用原生路径。
- 本机当前**未检测到任何 J-Link / ST-Link USB 设备**（`VID_1366` / `VID_0483` 均未出现），也还没装板子；上板验证待硬件接上后进行。
- 兼容克隆 J-Link 可能被 V8.18 驱动拒绝，遇到时换正版 / 旧版软件 / OpenOCD 路径。

硬件清单、SWD 接线表、BOOT0 设置、晶振与 LED 引脚核对项见 [README.md 第 1 节](../README.md)。

> `.vscode/tasks.json` 里的 `Flash: st-flash (BIN)` 是备用方案：winget 里没有 `stlink` / `STM32CubeProgrammer` 包，所以 `st-flash` **未安装**，该任务目前不可用。

## 复现安装命令

```powershell
winget install --id Arm.ArmGnuToolchain -e --accept-source-agreements --accept-package-agreements --silent
winget install --id Kitware.CMake -e --accept-source-agreements --accept-package-agreements --silent
winget install --id Ninja-build.Ninja -e --accept-source-agreements --accept-package-agreements --silent
winget install --id xpack-dev-tools.openocd-xpack -e --accept-source-agreements --accept-package-agreements --silent
winget install --id NordicSemiconductor.JLink -e --accept-source-agreements --accept-package-agreements --silent
powershell -ExecutionPolicy Bypass -File tools\setup-toolchain-path.ps1
```
