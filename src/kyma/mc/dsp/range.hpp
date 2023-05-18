#pragma once

#include <cmath>

namespace mc
{

template<typename T>
constexpr auto clamp(T in, T min, T max) noexcept -> T
{
    return std::fmin(std::fmax(in, min), max);
}

template<typename T>
constexpr auto mapToLinearRange(T in, T min, T max) noexcept -> T
{
    return clamp(min + in * (max - min), min, max);
}

}  // namespace mc
