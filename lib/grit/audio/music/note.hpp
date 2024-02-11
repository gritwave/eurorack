#pragma once

#include <etl/cmath.hpp>
#include <etl/concepts.hpp>

namespace grit {

/// \ingroup grit-audio-music
template<etl::floating_point Float>
[[nodiscard]] auto noteToHertz(Float note) -> Float
{
    static constexpr auto x = Float(1) / Float(12);
    return etl::pow(Float(2), (note - Float(69)) * x) * Float(440);
}

}  // namespace grit
