#pragma once

#include <etl/random.hpp>

namespace grit {

/// \brief Dummy dithering class. Performs no dithering.
template<typename URNG>
struct NoDither
{
    using seed_type = typename URNG::result_type;

    explicit constexpr NoDither(seed_type /*unused*/) {}

    [[nodiscard]] constexpr auto operator()(float v) -> float { return v; }
};

}  // namespace grit