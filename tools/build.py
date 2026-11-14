#!/usr/bin/env python3
"""Build the STM32F407 bare-metal firmware with zero build-system dependencies.

Requires only Python 3 and arm-none-eabi-gcc on PATH. Produces:
    build/direct/stm32f407-blink.elf / .hex / .bin / .map
    compile_commands.json          (for Cursor / clangd IntelliSense)

Usage:
    python tools/build.py                      # debug build, HSE = 8 MHz
    python tools/build.py --release            # -Os
    python tools/build.py --hse 25000000       # 25 MHz crystal
    python tools/build.py --clean
"""
from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BUILD = ROOT / "build" / "direct"
INC = ROOT / "inc"
LD = ROOT / "linker" / "STM32F407VGTx_FLASH.ld"
NAME = "stm32f407-blink"
SOURCES = [
    ROOT / "src" / "startup_stm32f407xx.c",
    ROOT / "src" / "system_stm32f4xx.c",
    ROOT / "src" / "main.c",
]
MCU_FLAGS = ["-mcpu=cortex-m4", "-mthumb", "-mfpu=fpv4-sp-d16", "-mfloat-abi=hard"]


def toolchain_bin(prefix: str) -> Path | None:
    """Locate the toolchain bin/ even when PATH has not been refreshed.

    Some installers (winget / NSIS) do not add the toolchain to PATH by default.
    ARM_TOOLCHAIN_BIN wins if set; otherwise common install roots are probed.
    """
    env = os.environ.get("ARM_TOOLCHAIN_BIN")
    if env:
        return Path(env)

    gcc_exe = prefix + "gcc" + (".exe" if os.name == "nt" else "")
    roots = [
        Path("C:/Program Files"),
        Path("C:/Program Files (x86)"),
        Path.home() / "AppData" / "Local" / "Programs",
        Path("C:/tools"),
        Path("/usr"),
        Path("/opt"),
    ]
    for root in roots:
        if not root.is_dir():
            continue
        try:
            # Bound the search: installers put bin/ at a shallow, known depth.
            for depth in (2, 3, 4):
                pattern = "/".join(["*"] * depth + ["bin", gcc_exe])
                hits = sorted(root.glob(pattern))
                if hits:
                    return hits[0].parent
        except OSError:
            pass
    return None


def find_tool(name: str, prefix: str, bin_dir: Path | None) -> str:
    exe = shutil.which(prefix + name)
    if exe is None and bin_dir is not None:
        candidate = bin_dir / (prefix + name + (".exe" if os.name == "nt" else ""))
        if candidate.is_file():
            exe = str(candidate)
    if exe is None:
        sys.exit(
            f"[ERROR] '{prefix}{name}' not found in PATH.\n"
            "        Install the Arm GNU Toolchain (arm-none-eabi) and add its bin/ to PATH.\n"
            "        https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads\n"
            "        Or set ARM_TOOLCHAIN_BIN to the toolchain's bin/ directory,\n"
            "        or point ARM_TOOLCHAIN_PREFIX at a custom toolchain prefix."
        )
    return exe


def run(cmd: list[str]) -> None:
    print("+", subprocess.list2cmdline(cmd), flush=True)
    subprocess.run(cmd, check=True, cwd=ROOT)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--release", action="store_true", help="Optimise for size (-Os)")
    ap.add_argument("--hse", type=int, default=8000000, help="HSE crystal frequency in Hz")
    ap.add_argument("--clean", action="store_true", help="Remove build/direct")
    ap.add_argument(
        "--prefix",
        default=os.environ.get("ARM_TOOLCHAIN_PREFIX", "arm-none-eabi-"),
        help="Toolchain prefix (default: arm-none-eabi-)",
    )
    args = ap.parse_args()

    if args.clean:
        shutil.rmtree(BUILD, ignore_errors=True)
        print("Removed", BUILD)
        return 0

    bin_dir = toolchain_bin(args.prefix) if shutil.which(args.prefix + "gcc") is None else None
    if bin_dir is not None:
        print("[info] using toolchain bin:", bin_dir)
    gcc = find_tool("gcc", args.prefix, bin_dir)
    objcopy = find_tool("objcopy", args.prefix, bin_dir)
    size = find_tool("size", args.prefix, bin_dir)
    BUILD.mkdir(parents=True, exist_ok=True)

    opt = "-Os" if args.release else "-Og"
    debug = "-g0" if args.release else "-g3"
    common = MCU_FLAGS + [
        "-std=gnu11",
        "-Wall",
        "-Wextra",
        "-ffunction-sections",
        "-fdata-sections",
        "-fno-common",
        opt,
        debug,
        f"-I{INC.as_posix()}",
        "-DSTM32F407XX",
        f"-DHSE_VALUE={args.hse}",
    ]

    compile_commands = []
    objects = []
    for src in SOURCES:
        obj = BUILD / (src.stem + ".o")
        cmd = [gcc, "-c", str(src), "-o", str(obj)] + common
        run(cmd)
        compile_commands.append(
            {"directory": str(ROOT), "command": subprocess.list2cmdline(cmd), "file": str(src)}
        )
        objects.append(str(obj))

    elf = BUILD / f"{NAME}.elf"
    link_cmd = (
        [gcc]
        + objects
        + [
            "-o", str(elf),
            *MCU_FLAGS,
            "-T", LD.as_posix(),
            "-nostartfiles",
            "-Wl,--gc-sections",
            f"-Wl,-Map={(BUILD / (NAME + '.map')).as_posix()}",
            "--specs=nano.specs",
            "--specs=nosys.specs",
        ]
    )
    run(link_cmd)

    run([objcopy, "-O", "ihex", str(elf), str(BUILD / f"{NAME}.hex")])
    run([objcopy, "-O", "binary", str(elf), str(BUILD / f"{NAME}.bin")])
    run([size, str(elf)])

    (ROOT / "compile_commands.json").write_text(
        json.dumps(compile_commands, indent=2), encoding="utf-8"
    )

    print("\n[OK] Artifacts in", BUILD)
    print("     ELF :", elf)
    print("     HEX :", BUILD / f"{NAME}.hex")
    print("     BIN :", BUILD / f"{NAME}.bin")
    print("     Map :", BUILD / f"{NAME}.map")
    return 0


if __name__ == "__main__":
    sys.exit(main())
