#pragma once

#include <grit/audio/waveshape/adaa1.hpp>

namespace grit {

struct FullWaveRectifierFunctions
{
    FullWaveRectifierFunctions() = default;

    [[nodiscard]] static auto f(etl::floating_point auto x) { return etl::abs(x); }

    [[nodiscard]] static auto AD1(etl::floating_point auto x)
    {
        using Float = decltype(x);
        return x <= Float(0) ? x * x * -0.5 : x * x * 0.5;
    }
};

template<etl::floating_point Float>
using FullWaveRectifierADAA1 = ADAA1<Float, FullWaveRectifierFunctions>;

}  // namespace grit
