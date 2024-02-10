#pragma once

#include <grit/audio/waveshape/adaa1.hpp>
#include <grit/audio/waveshape/wave_shaper.hpp>

namespace grit {

template<etl::floating_point Float>
struct FullWaveRectifierFunctions
{
    constexpr FullWaveRectifierFunctions() = default;

    [[nodiscard]] constexpr auto operator()(Float x) const { return f(x); }

    [[nodiscard]] static constexpr auto f(Float x) { return etl::abs(x); }

    [[nodiscard]] static constexpr auto ad1(Float x)
    {
        return x <= Float(0) ? x * x * Float(-0.5) : x * x * Float(0.5);
    }

    [[nodiscard]] static constexpr auto ad2(Float x)
    {
        auto const x3       = x * x * x;
        auto const oneSixth = Float(1) / Float(6);
        return x <= Float(0) ? x3 * -oneSixth : x3 * oneSixth;
    }
};

template<etl::floating_point Float>
using FullWaveRectifier = WaveShaper<Float, FullWaveRectifierFunctions<Float>>;

template<etl::floating_point Float>
using FullWaveRectifierADAA1 = ADAA1<Float, FullWaveRectifierFunctions<Float>>;

}  // namespace grit
