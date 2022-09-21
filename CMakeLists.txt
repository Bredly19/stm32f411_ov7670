cmake_minimum_required(VERSION 3.21)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
project(stm32f411_ov7670 C CXX ASM)

# specify cross compilers and tools
set(CMAKE_C_COMPILER arm-none-eabi-gcc.exe)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER  arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(SIZE arm-none-eabi-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)

# uncomment to mitigate c++17 absolute addresses warnings
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register")

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "Maximum optimization for speed")
    add_compile_options(-Ofast)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    message(STATUS "Maximum optimization for speed, debug info included")
    add_compile_options(-Ofast -g)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    message(STATUS "Maximum optimization for size")
    add_compile_options(-Os)
else ()
    message(STATUS "Minimal optimization, debug info included")
    add_compile_options(-Og -g)
endif ()

add_definitions(-DSTM32F40_41xxx)

include_directories(
        Drivers/CMSIS/Include
        Drivers/CMSIS/Device/ST/STM32F4xx
        Drivers/STM32F4xx_StdPeriph_Driver/inc
        Drivers/CMSIS/Device/ST/STM32F4xx/Config/Include
        ov7670fifo
)

list(
        APPEND SOURCES
        Src/main.c
        ov7670fifo/ov7670fifo.c
        Drivers/CMSIS/Device/ST/STM32F4xx/Startup_stm32f411xe/startup_stm32f411xe.S
)

list(
        APPEND SOURCES_STDPERIPH
        Drivers/CMSIS/Device/ST/STM32F4xx/Config/Source/system_stm32f4xx.c
        Drivers/STM32F4xx_StdPeriph_Driver/src/misc.c
        Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.c
        Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_i2c.c
        Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c
        Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c
        Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_usart.c
        Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.c
        Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.c
)
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Startup_stm32f411xe/STM32F411CEUx_FLASH.ld)

#Uncomment for hardware floating point
#add_compile_definitions(ARM_MATH_CM4;ARM_MATH_MATRIX_CHECK;ARM_MATH_ROUNDING)
#add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
#add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)

#Uncomment for software floating point
#add_compile_options(-mfloat-abi=soft)

add_compile_options(-mcpu=cortex-m4 -mtune=cortex-m4 -mthumb -mthumb-interwork)
add_compile_options(-ffunction-sections -fdata-sections -fno-common -fmessage-length=0)

add_compile_options($<$<COMPILE_LANGUAGE:ASM>:-x$<SEMICOLON>assembler-with-cpp>)

add_link_options(-Wl,-gc-sections,--print-memory-usage,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map)
add_link_options(-mcpu=cortex-m4 -mthumb -mthumb-interwork)
add_link_options(-T ${LINKER_SCRIPT})

add_executable(${PROJECT_NAME}.elf ${SOURCES} ${SOURCES_STDPERIPH} ${LINKER_SCRIPT})

set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE}
Building ${BIN_FILE}")
