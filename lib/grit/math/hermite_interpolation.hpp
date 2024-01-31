#pragma once

#include <etl/concepts.hpp>

namespace grit {

template<etl::floating_point Float>
[[nodiscard]] constexpr auto hermiteInterpolation(Float xm1, Float x0, Float x1, Float x2, Float pos) noexcept -> Float
{
    auto const slope0 = (x1 - xm1) * static_cast<Float>(0.5);
    auto const slope1 = (x2 - x0) * static_cast<Float>(0.5);

    auto const v    = x0 - x1;
    auto const w    = slope0 + v;
    auto const a    = w + v + slope1;
    auto const bNeg = w + a;

    auto const stage1 = a * pos - bNeg;
    auto const stage2 = stage1 * pos + slope0;

    return stage2 * pos + x0;
}

}  // namespace grit
