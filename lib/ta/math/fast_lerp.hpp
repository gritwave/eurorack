#pragma once

#include <etl/algorithm.hpp>
#include <etl/concepts.hpp>

namespace ta
{

template<etl::floating_point Float>
[[nodiscard]] constexpr auto fast_lerp(Float a, Float b, Float t) noexcept -> Float
{
    return a + (b - a) * t;
}

}  // namespace ta
