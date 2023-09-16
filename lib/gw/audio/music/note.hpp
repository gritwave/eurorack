#pragma once

#include <etl/cmath.hpp>

namespace gw {

template<typename T>
[[nodiscard]] auto noteToHertz(T note) noexcept -> T
{
    return etl::pow(T(2), (note - T(69)) / T(12)) * T(440);
}

}  // namespace gw
