# =============================================================================
#  Cortex-M3 / STM32F1 交叉编译工具链
#  默认使用 STM32CubeIDE 2.2.0 自带的 GNU Tools for STM32 (arm-none-eabi-gcc 14.3.1)
#  如果以后单独安装了 arm-none-eabi 工具链并加入 PATH，
#  可用 -DUSE_SYSTEM_ARM_GCC=ON 让 CMake 直接调用 PATH 里的 arm-none-eabi-gcc。
# =============================================================================

set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# 交叉编译时不做可执行文件链接测试（裸机工程无法在主机运行）
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

if(USE_SYSTEM_ARM_GCC)
  set(CMAKE_C_COMPILER   arm-none-eabi-gcc)
  set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
  set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
  set(CMAKE_AR           arm-none-eabi-ar)
  set(CMAKE_OBJCOPY      arm-none-eabi-objcopy)
  set(CMAKE_SIZE         arm-none-eabi-size)
else()
  # STM32CubeIDE 内置工具链根目录（可按实际版本号替换）
  set(ARM_TOOLCHAIN_ROOT
      "C:/ST/STM32CubeIDE_2.2.0/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.14.3.rel1.win32_1.0.100.202602081740/tools"
      CACHE PATH "ARM GNU 工具链根目录")

  set(CMAKE_C_COMPILER   "${ARM_TOOLCHAIN_ROOT}/bin/arm-none-eabi-gcc.exe")
  set(CMAKE_CXX_COMPILER "${ARM_TOOLCHAIN_ROOT}/bin/arm-none-eabi-g++.exe")
  set(CMAKE_ASM_COMPILER "${ARM_TOOLCHAIN_ROOT}/bin/arm-none-eabi-gcc.exe")
  set(CMAKE_AR           "${ARM_TOOLCHAIN_ROOT}/bin/arm-none-eabi-ar.exe")
  set(CMAKE_OBJCOPY      "${ARM_TOOLCHAIN_ROOT}/bin/arm-none-eabi-objcopy.exe")
  set(CMAKE_SIZE         "${ARM_TOOLCHAIN_ROOT}/bin/arm-none-eabi-size.exe")
endif()

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
