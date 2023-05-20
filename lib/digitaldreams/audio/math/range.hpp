#pragma once

#include <etl/algorithm.hpp>

namespace digitaldreams::audio
{

template<typename T>
[[nodiscard]] constexpr auto mapToRange(T in, T min, T max) noexcept -> T
{
    return etl::clamp(min + in * (max - min), min, max);
}

}  // namespace digitaldreams::audio
