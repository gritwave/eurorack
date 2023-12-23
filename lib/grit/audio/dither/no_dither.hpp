#pragma once

#include <etl/random.hpp>

namespace grit {

/// \brief Dummy dithering class. Performs no dithering.
template<typename URNG>
struct no_dither
{
    using seed_type = typename URNG::result_type;

    explicit constexpr no_dither(seed_type /*unused*/) noexcept {}

    [[nodiscard]] constexpr auto operator()(float v) noexcept -> float { return v; }
};

}  // namespace grit
