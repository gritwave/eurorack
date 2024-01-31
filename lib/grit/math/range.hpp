#pragma once

#include <etl/algorithm.hpp>
#include <etl/concepts.hpp>

namespace grit {

template<etl::floating_point Float>
[[nodiscard]] constexpr auto mapToRange(Float in, Float min, Float max) -> Float
{
    return etl::clamp(min + in * (max - min), min, max);
}

}  // namespace grit
