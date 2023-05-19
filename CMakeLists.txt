cmake_minimum_required(VERSION 3.23)
project(patch-init-firmware LANGUAGES C CXX)

find_program(CCACHE ccache)
if (CCACHE)
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
endif ()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(LIBDAISY_DIR ${CMAKE_SOURCE_DIR}/3rd_party/libDaisy)
set(LINKER_SCRIPT ${LIBDAISY_DIR}/core/STM32H750IB_flash.lds)

add_subdirectory(${LIBDAISY_DIR})
add_subdirectory(3rd_party/tetl/include)

add_subdirectory(lib)
add_subdirectory(src/kyma)
