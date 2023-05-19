#pragma once

#include <cmath>

namespace etl::audio
{

template<typename T>
[[nodiscard]] auto clamp(T in, T min, T max) noexcept -> T
{
    return std::fmin(std::fmax(in, min), max);
}

template<typename T>
[[nodiscard]] auto mapToRange(T in, T min, T max) noexcept -> T
{
    return clamp(min + in * (max - min), min, max);
}

}  // namespace etl::audio
