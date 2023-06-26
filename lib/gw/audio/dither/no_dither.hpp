#pragma once

#include <etl/random.hpp>

namespace gw
{

/// \brief Dummy dithering class. Performs no dithering.
template<typename URNG>
struct NoDither
{
    using seed_type = typename URNG::result_type;

    explicit constexpr NoDither(seed_type /*unused*/) noexcept {}

    [[nodiscard]] constexpr auto operator()(float v) noexcept -> float { return v; }
};

}  // namespace gw
