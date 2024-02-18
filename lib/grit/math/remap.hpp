#pragma once

#include <etl/algorithm.hpp>
#include <etl/concepts.hpp>

namespace grit {

/// \ingroup grit-math
template<etl::floating_point Float>
[[nodiscard]] constexpr auto remap(Float normalized, Float min, Float max) -> Float
{
    return min + normalized * (max - min);
}

/// \ingroup grit-math
template<etl::floating_point Float>
[[nodiscard]] constexpr auto remap(Float src, Float srcMin, Float srcMax, Float destMin, Float destMax) -> Float
{
    return destMin + ((destMax - destMin) * (src - srcMin)) / (srcMax - srcMin);
}

}  // namespace grit
