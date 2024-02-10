#pragma once

#include <grit/audio/waveshape/wave_shaper.hpp>
#include <grit/audio/waveshape/wave_shaper_adaa1.hpp>
#include <grit/math/sign.hpp>

#include <etl/algorithm.hpp>

namespace grit {

template<etl::floating_point Float>
struct HardClipperFunctions
{
    constexpr HardClipperFunctions() = default;

    [[nodiscard]] constexpr auto operator()(Float x) const { return f(x); }

    [[nodiscard]] static constexpr auto f(Float x) { return etl::clamp(x, Float(-1), Float(+1)); }

    [[nodiscard]] static constexpr auto ad1(Float x)
    {
        return etl::abs(x) > Float(1) ? x * sign(x) : (x * x) * Float(0.5);
    }
};

template<etl::floating_point Float>
using HardClipper = WaveShaper<Float, HardClipperFunctions<Float>>;

template<etl::floating_point Float>
using HardClipperADAA1 = WaveShaperADAA1<Float, HardClipperFunctions<Float>>;

}  // namespace grit
