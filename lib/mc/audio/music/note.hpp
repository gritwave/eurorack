#pragma once

#include <cmath>

namespace mc::audio
{

template<typename T>
[[nodiscard]] auto noteToHertz(T note) noexcept -> T
{
    return std::pow(T(2), (note - T(69)) / T(12)) * T(440);
}

}  // namespace mc::audio
