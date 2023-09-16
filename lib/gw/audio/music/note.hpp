#pragma once

#include <etl/cmath.hpp>
#include <etl/concepts.hpp>

namespace gw {

template<etl::floating_point Float>
[[nodiscard]] auto noteToHertz(Float note) noexcept -> Float
{
    static constexpr auto x = Float(1) / Float(12);
    return etl::pow(Float(2), (note - Float(69)) * x) * Float(440);
}

}  // namespace gw
