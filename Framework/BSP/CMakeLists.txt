# =============================================================================
#  Helios 入队题  |  STM32F103C8T6  |  Cortex-M3  |  CMake + Ninja + GCC
#  本文件位于 Framework/BSP，负责：HAL 库 + 启动/时钟/中断 + 链接脚本,
#  同时自动把上层业务代码（Framework/Application、Framework/Module、
#  二轴自稳云台）一起编译进来。
#
#  用法：
#     cmake -S Framework/BSP -B Framework/BSP/build -G Ninja -DCMAKE_BUILD_TYPE=Debug
#     cmake --build Framework/BSP/build
# =============================================================================
cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/cmake/gcc-arm-none-eabi.cmake"
      CACHE FILEPATH "交叉编译工具链文件")
endif()

project(helios_f103 C CXX ASM)

# ---------------------------------------------------------------- 路径
set(BSP_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(CONTEST_ROOT "${BSP_DIR}/../.." ABSOLUTE)   # 仓库根目录
message(STATUS "仓库根目录: ${CONTEST_ROOT}")

# ---------------------------------------------------------------- 基本参数
set(MCU_DEVICE "STM32F103xB" CACHE STRING "芯片宏定义，决定启动文件与内存布局")
string(TOLOWER "${MCU_DEVICE}" MCU_LOWER)
string(REGEX REPLACE "^stm32f103" "" MCU_CODE "${MCU_LOWER}")
set(STARTUP_FILE "startup_stm32f103${MCU_CODE}.s")

# ---------------------------------------------------------------- HAL / CMSIS
set(HAL_DIR "${BSP_DIR}/Drivers" CACHE PATH "HAL/CMSIS 根目录（Drivers）")
if(NOT EXISTS "${HAL_DIR}/STM32F1xx_HAL_Driver/Inc/stm32f1xx_hal.h")
  message(FATAL_ERROR "没找到 HAL 库，请检查 ${HAL_DIR}")
endif()
message(STATUS "HAL 库目录: ${HAL_DIR}")

set(CMSIS_DIR     "${HAL_DIR}/CMSIS")
set(DEVICE_DIR    "${CMSIS_DIR}/Device/ST/STM32F1xx")
set(TEMPLATES_DIR "${DEVICE_DIR}/Source/Templates")

# ---------------------------------------------------------------- 链接脚本
set(LD_SCRIPT "${BSP_DIR}/STM32F103C8TX_FLASH.ld" CACHE FILEPATH "链接脚本")
if(NOT EXISTS "${LD_SCRIPT}")
  message(FATAL_ERROR "链接脚本不存在: ${LD_SCRIPT}")
endif()

# ---------------------------------------------------------------- 源文件
# HAL 驱动（*_template.c 会与自己的 Hsp/Timebase 实现重复定义，必须排除）
file(GLOB HAL_SOURCES CONFIGURE_DEPENDS "${HAL_DIR}/STM32F1xx_HAL_Driver/Src/*.c")
foreach(_hal_src ${HAL_SOURCES})
  get_filename_component(_hal_name "${_hal_src}" NAME)
  if(_hal_name MATCHES "_template.c$")
    list(REMOVE_ITEM HAL_SOURCES "${_hal_src}")
  endif()
endforeach()
list(LENGTH HAL_SOURCES HAL_SRC_COUNT)
message(STATUS "HAL 源文件数量: ${HAL_SRC_COUNT}")

# BSP 自有源码：Core（main / it / msp）+ bsp_xxx 外设封装
# 注意：BSP 根目录用非递归 GLOB，避免把 build/ 里的 CMake 自检文件收进来
file(GLOB BSP_ROOT_SOURCES CONFIGURE_DEPENDS
     "${BSP_DIR}/*.c"
     "${BSP_DIR}/*.cpp")
file(GLOB_RECURSE BSP_CORE_SOURCES CONFIGURE_DEPENDS
     "${BSP_DIR}/Core/Src/*.c"
     "${BSP_DIR}/Core/Src/*.cpp")
set(BSP_SOURCES ${BSP_ROOT_SOURCES} ${BSP_CORE_SOURCES})

# 上层业务源码：Application / Module / 二轴自稳云台（递归，自动收新文件）
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
     "${CONTEST_ROOT}/Framework/Application/*.c"
     "${CONTEST_ROOT}/Framework/Application/*.cpp"
     "${CONTEST_ROOT}/Framework/Module/*.c"
     "${CONTEST_ROOT}/Framework/Module/*.cpp"
     "${CONTEST_ROOT}/二轴自稳云台/*.c"
     "${CONTEST_ROOT}/二轴自稳云台/*.cpp")
# 保险：剔除任何 build 目录下的产物文件
list(FILTER BSP_SOURCES EXCLUDE REGEX "/build/")
list(FILTER APP_SOURCES EXCLUDE REGEX "/build/")

set(CMSIS_SOURCES   "${TEMPLATES_DIR}/system_stm32f1xx.c")
set(STARTUP_SOURCE  "${TEMPLATES_DIR}/gcc/${STARTUP_FILE}")

# ---------------------------------------------------------------- 目标
add_executable(${PROJECT_NAME}
    ${HAL_SOURCES}
    ${CMSIS_SOURCES}
    ${STARTUP_SOURCE}
    ${BSP_SOURCES}
    ${APP_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${HAL_DIR}/STM32F1xx_HAL_Driver/Inc"
    "${DEVICE_DIR}/Include"
    "${CMSIS_DIR}/Include"
    "${BSP_DIR}/Core/Inc"
    "${BSP_DIR}"
    "${CONTEST_ROOT}/Framework"
    "${CONTEST_ROOT}/Framework/Application"
    "${CONTEST_ROOT}/Framework/Module"
    "${CONTEST_ROOT}/二轴自稳云台"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    USE_HAL_DRIVER
    ${MCU_DEVICE}
)

# ---------------------------------------------------------------- 编译选项
set(CPU_FLAGS "-mcpu=cortex-m3;-mthumb;-mfloat-abi=soft")

target_compile_options(${PROJECT_NAME} PRIVATE
    ${CPU_FLAGS}
    -Wall
    -fno-common
    -ffunction-sections
    -fdata-sections
)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++17;-fno-exceptions;-fno-rtti;-fno-threadsafe-statics;-fno-use-cxa-atexit>
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(${PROJECT_NAME} PRIVATE -Og -g3 -gdwarf-2)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -O2 -g)
endif()

# ---------------------------------------------------------------- 链接选项
target_link_options(${PROJECT_NAME} PRIVATE
    ${CPU_FLAGS}
    -T${LD_SCRIPT}
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.map
    -Wl,--print-memory-usage
    --specs=nano.specs
    --specs=nosys.specs
)

target_link_libraries(${PROJECT_NAME} PRIVATE m)

# ---------------------------------------------------------------- 产物转换
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>
    COMMENT "生成 .hex / .bin 并统计内存占用"
)
