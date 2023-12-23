set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER_ID GNU)
set(CMAKE_CXX_COMPILER_ID GNU)

set(CMAKE_C_COMPILER    "arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER  "arm-none-eabi-g++")
set(CMAKE_LINKER        "arm-none-eabi-gcc")
set(CMAKE_AR            "arm-none-eabi-gcc-ar")
set(CMAKE_RANLIB        "arm-none-eabi-gcc-ranlib")
set(CMAKE_OBJCOPY       "arm-none-eabi-objcopy")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(MCU "-mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16")
set(OBJECT_GEN_FLAGS "${MCU} -fno-exceptions -Wall -Wextra -ffast-math -ffunction-sections -fdata-sections -fomit-frame-pointer -finline-functions -Wno-attributes -Wno-strict-aliasing -Wno-maybe-uninitialized -Wno-missing-attributes -Wno-stringop-overflow -Wno-unused-parameter -Wno-vla -Wno-missing-field-initializers")
set(CMAKE_C_FLAGS   "${OBJECT_GEN_FLAGS} " CACHE INTERNAL "C Compiler options")
set(CMAKE_CXX_FLAGS "${OBJECT_GEN_FLAGS} -Wno-register" CACHE INTERNAL "C++ Compiler options")
set(CMAKE_ASM_FLAGS "${OBJECT_GEN_FLAGS} -x assembler-with-cpp " CACHE INTERNAL "ASM Compiler options")

add_compile_definitions(
    CORE_CM7
    STM32H750xx
    STM32H750IB
    ARM_MATH_CM7
    flash_layout
    HSE_VALUE=16000000
    USE_HAL_DRIVER
    USE_FULL_LL_DRIVER
    TETL_NDEBUG
)


# -Wl,--gc-sections     Perform the dead code elimination.
# --specs=nano.specs    Link with newlib-nano.
# --specs=nosys.specs   No syscalls, provide empty implementations for the POSIX system calls.
set(CMAKE_EXE_LINKER_FLAGS "${MCU} -Wl,--gc-sections --specs=nano.specs --specs=nosys.specs -Wl,-Map=${CMAKE_PROJECT_NAME}.map" CACHE INTERNAL "Linker options")

#---------------------------------------------------------------------------------------
# Set debug/release build configuration Options
#---------------------------------------------------------------------------------------

# Options for DEBUG build
# -Og   Enables optimizations that do not interfere with debugging.
# -g    Produce debugging information in the operating system’s native format.
set(CMAKE_C_FLAGS_DEBUG "-Og -g3" CACHE INTERNAL "C Compiler options for debug build type")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g3" CACHE INTERNAL "C++ Compiler options for debug build type")
set(CMAKE_ASM_FLAGS_DEBUG "-g3" CACHE INTERNAL "ASM Compiler options for debug build type")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "" CACHE INTERNAL "Linker options for debug build type")

# Options for RELEASE build
# -Os   Optimize for size. -Os enables all -O2 optimizations.
# -flto Runs the standard link-time optimizer.
set(CMAKE_C_FLAGS_RELEASE "-O3 " CACHE INTERNAL "C Compiler options for release build type")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 " CACHE INTERNAL "C++ Compiler options for release build type")
set(CMAKE_ASM_FLAGS_RELEASE "" CACHE INTERNAL "ASM Compiler options for release build type")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "" CACHE INTERNAL "Linker options for release build type")

set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -g3" CACHE INTERNAL "C Compiler options for release build type")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g3" CACHE INTERNAL "C++ Compiler options for release build type")
set(CMAKE_ASM_FLAGS_RELWITHDEBINFO "-g3" CACHE INTERNAL "ASM Compiler options for release build type")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "" CACHE INTERNAL "Linker options for release build type")
