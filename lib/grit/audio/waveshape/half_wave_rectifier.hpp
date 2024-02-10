#pragma once

#include <grit/audio/waveshape/wave_shaper.hpp>
#include <grit/audio/waveshape/wave_shaper_adaa1.hpp>

namespace grit {

template<etl::floating_point Float>
struct HalfWaveRectifierFunctions
{
    constexpr HalfWaveRectifierFunctions() = default;

    [[nodiscard]] constexpr auto operator()(Float x) const { return f(x); }

    [[nodiscard]] static constexpr auto f(Float x) { return x > Float(0) ? x : Float(0); }

    [[nodiscard]] static constexpr auto ad1(Float x) { return x <= Float(0) ? Float(0) : x * x * 0.5; }

    [[nodiscard]] static constexpr auto ad2(Float x)
    {
        return x <= Float(0) ? Float(0) : x * x * x * (Float(1) / Float(6));
    }
};

template<etl::floating_point Float>
using HalfWaveRectifier = WaveShaper<Float, HalfWaveRectifierFunctions<Float>>;

template<etl::floating_point Float>
using HalfWaveRectifierADAA1 = WaveShaperADAA1<Float, HalfWaveRectifierFunctions<Float>>;

}  // namespace grit
