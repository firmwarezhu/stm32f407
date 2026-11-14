cmake_minimum_required(VERSION 3.21)

project(stm32f407_blink C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)          # gnu11: __asm__ / __attribute__ are used
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# Target
# ---------------------------------------------------------------------------
add_executable(firmware
    src/startup_stm32f407xx.c
    src/system_stm32f4xx.c
    src/main.c
)

set_target_properties(firmware PROPERTIES
    OUTPUT_NAME "stm32f407-blink"
    SUFFIX ".elf"
)

target_include_directories(firmware PRIVATE inc)

target_compile_definitions(firmware PRIVATE
    STM32F407XX
    HSE_VALUE=8000000
)

set(MCU_FLAGS
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
)

target_compile_options(firmware PRIVATE
    ${MCU_FLAGS}
    -Wall
    -Wextra
    -ffunction-sections
    -fdata-sections
    -fno-common
    $<$<CONFIG:Debug>:-Og>
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Release>:-Os>
)

target_link_options(firmware PRIVATE
    ${MCU_FLAGS}
    -T "${CMAKE_SOURCE_DIR}/linker/STM32F407VGTx_FLASH.ld"
    -nostartfiles
    -Wl,--gc-sections
    "-Wl,-Map=${CMAKE_BINARY_DIR}/stm32f407-blink.map"
    --specs=nano.specs
    --specs=nosys.specs
)

# ---------------------------------------------------------------------------
# Post-build artifacts: .hex / .bin / size report
# ---------------------------------------------------------------------------
add_custom_command(TARGET firmware POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex  $<TARGET_FILE:firmware> ${CMAKE_BINARY_DIR}/stm32f407-blink.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:firmware> ${CMAKE_BINARY_DIR}/stm32f407-blink.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:firmware>
    COMMENT "Generating HEX/BIN and printing section sizes"
    VERBATIM
)

# Make IntelliSense / clangd work from the project root regardless of preset.
add_custom_target(export_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_SOURCE_DIR}/compile_commands.json
    DEPENDS firmware
    COMMENT "Exporting compile_commands.json to project root"
    VERBATIM
)
