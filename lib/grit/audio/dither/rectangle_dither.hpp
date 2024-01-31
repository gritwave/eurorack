#pragma once

#include <etl/random.hpp>

namespace grit {

template<typename URNG>
struct RectangleDither
{
    using seed_type = typename URNG::result_type;

    explicit constexpr RectangleDither(seed_type seed) noexcept : _urng{seed} {}

    [[nodiscard]] constexpr auto operator()(float v) noexcept -> float { return v + _dist(_urng); }

private:
    URNG _urng;
    etl::uniform_real_distribution<float> _dist{-0.5F, 0.5F};
};

}  // namespace grit
