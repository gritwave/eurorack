cmake_minimum_required(VERSION 3.23...3.27)

file(STRINGS VERSION CURRENT_VERSION)
project(grit-eurorack-dev VERSION ${CURRENT_VERSION} LANGUAGES C CXX)

option(GRITWAVE_EURORACK_ENABLE_PLUGIN "Build plugin (development tool)" OFF)

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
    include(FetchContent)
    FetchContent_Declare(Catch2 GIT_REPOSITORY "https://github.com/catchorg/Catch2" GIT_TAG "v3.5.2" GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(Catch2)
    include(${Catch2_SOURCE_DIR}/extras/Catch.cmake)

    add_executable(grit-eurorack-tests)
    catch_discover_tests(grit-eurorack-tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    target_link_libraries(grit-eurorack-tests PRIVATE gritwave::eurorack Catch2::Catch2WithMain)
    target_compile_options(grit-eurorack-tests PRIVATE "-Wall" "-Wextra" "-Wpedantic")
    target_sources(grit-eurorack-tests
        PRIVATE
            "lib/grit/audio_test.cpp"
            "lib/grit/fft_test.cpp"
            "lib/grit/math_test.cpp"
            "lib/grit/unit_test.cpp"

            "lib/grit/audio/delay/static_delay_line_test.cpp"
            "lib/grit/audio/dynamic/transient_shaper_test.cpp"
            "lib/grit/audio/envelope/envelope_follower_test.cpp"
            "lib/grit/audio/music/note_test.cpp"
            "lib/grit/audio/noise/airwindows_vinyl_dither_test.cpp"
            "lib/grit/audio/noise/dither_test.cpp"
            "lib/grit/audio/noise/white_noise_test.cpp"
            "lib/grit/audio/stereo/stereo_frame_test.cpp"
            "lib/grit/audio/waveshape/diode_rectifier_test.cpp"
            "lib/grit/audio/waveshape/full_wave_rectifier_test.cpp"
            "lib/grit/audio/waveshape/half_wave_rectifier_test.cpp"
            "lib/grit/audio/waveshape/hard_clipper_test.cpp"
            "lib/grit/audio/waveshape/tanh_clipper_test.cpp"
            "lib/grit/audio/waveshape/wave_shaper_test.cpp"
            "lib/grit/audio/waveshape/wave_shaper_adaa1_test.cpp"
            "lib/grit/eurorack/hades_test.cpp"
            "lib/grit/fft/fft_test.cpp"
            "lib/grit/math/ilog2_test.cpp"
            "lib/grit/math/ipow_test.cpp"
            "lib/grit/math/power_test.cpp"
            "lib/grit/math/normalizable_range_test.cpp"
            "lib/grit/math/static_lookup_table_test.cpp"
            "lib/grit/unit/decibel_test.cpp"
    )

    if(GRITWAVE_EURORACK_ENABLE_PLUGIN)
        add_subdirectory(src/plugin)
    endif()
endif()
