cmake_minimum_required(VERSION 3.23...3.27)
project(gw-eurorack)

add_library(gw-eurorack INTERFACE)
add_library(gw::eurorack ALIAS gw-eurorack)
target_compile_features(gw-eurorack INTERFACE cxx_std_20)
target_link_libraries(gw-eurorack INTERFACE tetl::etl)

target_sources(gw-eurorack INTERFACE
    FILE_SET
        HEADERS
    BASE_DIRS
        ${PROJECT_SOURCE_DIR}
    FILES
        "gw/audio/delay/delay_line.hpp"
        "gw/audio/delay/static_delay_line.hpp "
        "gw/audio/dither/no_dither.hpp"
        "gw/audio/dither/rectangle_dither.hpp"
        "gw/audio/dither/triangle_dither.hpp"
        "gw/audio/dynamic/compressor.hpp"
        "gw/audio/envelope/adsr.hpp"
        "gw/audio/mix/cross_fade.hpp"
        "gw/audio/music/note.hpp"
        "gw/audio/noise/white_noise.hpp"
        "gw/audio/oscillator/oscillator.hpp"
        "gw/audio/oscillator/variable_shape_oscillator.hpp"
        "gw/audio/oscillator/wavetable_oscillator.hpp"
        "gw/audio/unit/time.hpp"
        "gw/audio/waveshape/wave_shaper.hpp"

        "gw/core/arm.hpp"
        "gw/core/benchmark.hpp"
        "gw/core/config.hpp"
        "gw/core/mdspan.hpp"

        "gw/eurorack/hades.hpp"

        "gw/fft/bitrevorder.hpp"
        "gw/fft/direction.hpp"
        "gw/fft/fft.hpp"

        "gw/math/buffer_interpolation.hpp"
        "gw/math/dynamic_smoothing.hpp"
        "gw/math/fast_lerp.hpp"
        "gw/math/hermite_interpolation.hpp"
        "gw/math/ilog2.hpp"
        "gw/math/ipow.hpp"
        "gw/math/range.hpp"

        "gw/testing/approx.hpp"
)
