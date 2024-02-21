cmake_minimum_required(VERSION 3.23...3.27)
project(gritwave-grit)

add_library(gritwave-grit INTERFACE)
add_library(gritwave::grit ALIAS gritwave-grit)
target_compile_features(gritwave-grit INTERFACE cxx_std_20)
target_link_libraries(gritwave-grit INTERFACE tetl::etl)

if(MSVC)
  target_compile_options(gritwave-grit INTERFACE "/permissive-" "/Zc:__cplusplus")
endif(MSVC)

target_sources(gritwave-grit INTERFACE
    FILE_SET
        HEADERS
    BASE_DIRS
        ${PROJECT_SOURCE_DIR}
    FILES
        "grit/audio.hpp"

        "grit/audio/airwindows.hpp"
        "grit/audio/airwindows/airwindows_fire_amp.hpp"
        "grit/audio/airwindows/airwindows_grind_amp.hpp"
        "grit/audio/airwindows/airwindows_vinyl_dither.hpp"

        "grit/audio/delay.hpp"
        "grit/audio/delay/non_owning_delay_line.hpp"
        "grit/audio/delay/static_delay_line.hpp"

        "grit/audio/dynamic.hpp"
        "grit/audio/dynamic/compressor.hpp"
        "grit/audio/dynamic/dynamic.hpp"
        "grit/audio/dynamic/gain_computer.hpp"
        "grit/audio/dynamic/level_detector.hpp"
        "grit/audio/dynamic/transient_shaper.hpp"

        "grit/audio/envelope.hpp"
        "grit/audio/envelope/envelope_adsr.hpp"
        "grit/audio/envelope/envelope_follower.hpp"

        "grit/audio/filter.hpp"
        "grit/audio/filter/dynamic_smoothing.hpp"

        "grit/audio/mix.hpp"
        "grit/audio/mix/cross_fade.hpp"

        "grit/audio/music.hpp"
        "grit/audio/music/note.hpp"

        "grit/audio/noise.hpp"
        "grit/audio/noise/dither.hpp"
        "grit/audio/noise/white_noise.hpp"

        "grit/audio/oscillator.hpp"
        "grit/audio/oscillator/oscillator.hpp"
        "grit/audio/oscillator/variable_shape_oscillator.hpp"
        "grit/audio/oscillator/wavetable_oscillator.hpp"

        "grit/audio/stereo.hpp"
        "grit/audio/stereo/mid_side_frame.hpp"
        "grit/audio/stereo/stereo_block.hpp"
        "grit/audio/stereo/stereo_frame.hpp"
        "grit/audio/stereo/stereo_width.hpp"

        "grit/audio/waveshape.hpp"
        "grit/audio/waveshape/diode_rectifier.hpp"
        "grit/audio/waveshape/full_wave_rectifier.hpp"
        "grit/audio/waveshape/half_wave_rectifier.hpp"
        "grit/audio/waveshape/hard_clipper.hpp"
        "grit/audio/waveshape/tanh_clipper.hpp"
        "grit/audio/waveshape/wave_shaper.hpp"
        "grit/audio/waveshape/wave_shaper_adaa1.hpp"

        "grit/core/arm.hpp"
        "grit/core/benchmark.hpp"
        "grit/core/config.hpp"
        "grit/core/mdspan.hpp"

        "grit/fft.hpp"
        "grit/fft/bitrevorder.hpp"
        "grit/fft/direction.hpp"
        "grit/fft/fft.hpp"

        "grit/math.hpp"
        "grit/math/buffer_interpolation.hpp"
        "grit/math/hermite_interpolation.hpp"
        "grit/math/ilog2.hpp"
        "grit/math/ipow.hpp"
        "grit/math/linear_interpolation.hpp"
        "grit/math/normalizable_range.hpp"
        "grit/math/power.hpp"
        "grit/math/remap.hpp"
        "grit/math/sign.hpp"
        "grit/math/static_lookup_table.hpp"
        "grit/math/static_lookup_table_transform.hpp"
        "grit/math/trigonometry.hpp"

        "grit/unit.hpp"
        "grit/unit/decibel.hpp"
        "grit/unit/time.hpp"
)

add_library(gritwave-eurorack)
add_library(gritwave::eurorack ALIAS gritwave-eurorack)
target_link_libraries(gritwave-eurorack PUBLIC gritwave::grit)
target_sources(gritwave-eurorack
    PRIVATE
        "grit/eurorack.hpp"

        "grit/eurorack/ares.cpp"
        "grit/eurorack/ares.hpp"
        "grit/eurorack/kyma.cpp"
        "grit/eurorack/kyma.hpp"
        "grit/eurorack/poseidon.cpp"
        "grit/eurorack/poseidon.hpp"
)
