#pragma once

#include <etl/cmath.hpp>
#include <etl/numbers.hpp>

namespace grit {

/// \ingroup grit-math
template<etl::floating_point Float>
[[nodiscard]] constexpr auto bhaskara(Float x) -> Float
{
    auto const pi    = static_cast<Float>(etl::numbers::pi);
    auto const num   = Float(16) * x * (pi - x);
    auto const denom = Float(5) * pi * pi - Float(4) * x * (pi - x);
    return num / denom;
}

}  // namespace grit
