#pragma once

#include <etl/algorithm.hpp>
#include <etl/concepts.hpp>

namespace grit {

/// \ingroup grit-math
template<etl::floating_point Float>
[[nodiscard]] constexpr auto linearInterpolation(Float a, Float b, Float t) -> Float
{
    return a + (b - a) * t;
}

}  // namespace grit
