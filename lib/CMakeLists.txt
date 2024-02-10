cmake_minimum_required(VERSION 3.23...3.27)
project(grit-eurorack)

add_library(grit-eurorack INTERFACE)
add_library(grit::eurorack ALIAS grit-eurorack)
target_compile_features(grit-eurorack INTERFACE cxx_std_20)
target_link_libraries(grit-eurorack INTERFACE tetl::etl)

target_sources(grit-eurorack INTERFACE
    FILE_SET
        HEADERS
    BASE_DIRS
        ${PROJECT_SOURCE_DIR}
    FILES
        "grit/audio/delay/delay_line.hpp"
        "grit/audio/delay/static_delay_line.hpp "
        "grit/audio/dynamic/compressor.hpp"
        "grit/audio/envelope/adsr.hpp"
        "grit/audio/envelope/envelope_follower.hpp"
        "grit/audio/filter/dynamic_smoothing.hpp"
        "grit/audio/mix/cross_fade.hpp"
        "grit/audio/music/note.hpp"
        "grit/audio/noise/dither.hpp"
        "grit/audio/noise/white_noise.hpp"
        "grit/audio/oscillator/oscillator.hpp"
        "grit/audio/oscillator/variable_shape_oscillator.hpp"
        "grit/audio/oscillator/wavetable_oscillator.hpp"
        "grit/audio/waveshape/adaa1.hpp"
        "grit/audio/waveshape/full_wave_rectifier.hpp"
        "grit/audio/waveshape/half_wave_rectifier.hpp"
        "grit/audio/waveshape/hard_clipper.hpp"
        "grit/audio/waveshape/wave_shaper.hpp"

        "grit/core/arm.hpp"
        "grit/core/benchmark.hpp"
        "grit/core/config.hpp"
        "grit/core/mdspan.hpp"

        "grit/eurorack/hades.hpp"

        "grit/fft/bitrevorder.hpp"
        "grit/fft/direction.hpp"
        "grit/fft/fft.hpp"

        "grit/math/buffer_interpolation.hpp"
        "grit/math/hermite_interpolation.hpp"
        "grit/math/ilog2.hpp"
        "grit/math/ipow.hpp"
        "grit/math/linear_interpolation.hpp"
        "grit/math/remap.hpp"
        "grit/math/sign.hpp"
        "grit/math/static_lookup_table.hpp"
        "grit/math/static_lookup_table.hpp"

        "grit/testing/approx.hpp"

        "grit/unit/decibel.hpp"
        "grit/unit/time.hpp"
)
