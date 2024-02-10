#pragma once

#include <grit/audio/waveshape/adaa1.hpp>
#include <grit/audio/waveshape/wave_shaper.hpp>

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
        if (x < Float(-1)) {
            return -x;
        }
        if (x > Float(+1)) {
            return x;
        }
        return (x * x) / Float(2);
    }
};

template<etl::floating_point Float>
using HardClipper = WaveShaper<Float, HardClipperFunctions<Float>>;

template<etl::floating_point Float>
using HardClipperADAA1 = ADAA1<Float, HardClipperFunctions<Float>>;

}  // namespace grit
