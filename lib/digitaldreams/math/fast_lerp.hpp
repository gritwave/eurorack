#pragma once

#include <etl/algorithm.hpp>

namespace digitaldreams
{

template<typename Float>
[[nodiscard]] constexpr auto fast_lerp(Float a, Float b, Float t) noexcept -> Float
{
    return a + (b - a) * t;
}

}  // namespace digitaldreams
