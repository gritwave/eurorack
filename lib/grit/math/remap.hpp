#pragma once

#include <etl/algorithm.hpp>
#include <etl/concepts.hpp>

namespace grit {

/// \ingroup grit-math
template<etl::floating_point Float>
[[nodiscard]] constexpr auto remap(Float in, Float min, Float max) -> Float
{
    return etl::clamp(min + in * (max - min), min, max);
}

/// \ingroup grit-math
template<etl::floating_point Float>
[[nodiscard]] constexpr auto remap(Float in, Float srcMin, Float srcMax, Float destMin, Float destMax) -> Float
{
    return destMin + ((destMax - destMin) * (in - srcMin)) / (srcMax - srcMin);
}

}  // namespace grit
