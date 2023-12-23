cmake_minimum_required(VERSION 3.23...3.27)

file(STRINGS VERSION CURRENT_VERSION)
project(patch-init-firmware VERSION ${CURRENT_VERSION} LANGUAGES C CXX)

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

enable_testing()
include(CTest)
include("cmake/add_baremetal_executable.cmake")

add_subdirectory(3rd_party/tetl/include)
add_subdirectory(lib)

if(CMAKE_CROSSCOMPILING)
    add_subdirectory(${LIBDAISY_DIR})
    add_subdirectory(src/astra)
    add_subdirectory(src/benchmark)
    add_subdirectory(src/hades)
    add_subdirectory(src/kyma)
endif()

if(NOT CMAKE_CROSSCOMPILING)
    add_executable(gw-eurorack-tests)
    add_test(NAME gw-eurorack-tests COMMAND gw-eurorack-tests)
    target_link_libraries(gw-eurorack-tests PRIVATE gw::eurorack)
    target_compile_options(gw-eurorack-tests PRIVATE "-Wall" "-Wextra" "-Wpedantic")
    target_sources(gw-eurorack-tests
        PRIVATE
            "lib/main_test.cpp"

            "lib/gw/audio/delay/static_delay_line_test.cpp"
            "lib/gw/audio/dither/no_dither_test.cpp"
            "lib/gw/audio/dither/rectangle_dither_test.cpp"
            "lib/gw/audio/dither/triangle_dither_test.cpp"
            "lib/gw/audio/envelope/envelope_follower_test.cpp"
            "lib/gw/audio/music/note_test.cpp"
            "lib/gw/audio/noise/white_noise_test.cpp"
            "lib/gw/audio/unit/decibel_test.cpp"
            "lib/gw/fft/fft_test.cpp"
            "lib/gw/math/ilog2_test.cpp"
            "lib/gw/math/ipow_test.cpp"
    )
endif()
