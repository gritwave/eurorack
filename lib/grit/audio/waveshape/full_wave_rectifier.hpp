#pragma once

#include <grit/audio/waveshape/adaa1.hpp>
#include <grit/audio/waveshape/wave_shaper.hpp>
#include <grit/math/sign.hpp>

namespace grit {

template<etl::floating_point Float>
struct FullWaveRectifierFunctions
{
    constexpr FullWaveRectifierFunctions() = default;

    [[nodiscard]] constexpr auto operator()(Float x) const { return f(x); }

    [[nodiscard]] static constexpr auto f(Float x) { return etl::abs(x); }

    [[nodiscard]] static constexpr auto ad1(Float x) { return x * x * Float(0.5) * sign(x); }

    [[nodiscard]] static constexpr auto ad2(Float x) { return x * x * x * (Float(1) / Float(6)) * sign(x); }
};

template<etl::floating_point Float>
using FullWaveRectifier = WaveShaper<Float, FullWaveRectifierFunctions<Float>>;

template<etl::floating_point Float>
using FullWaveRectifierADAA1 = ADAA1<Float, FullWaveRectifierFunctions<Float>>;

}  // namespace grit
