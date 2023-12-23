#pragma once

#include <etl/random.hpp>

namespace grit {

template<typename URNG>
struct triangle_dither
{
    using seed_type = typename URNG::result_type;

    explicit constexpr triangle_dither(seed_type seed) noexcept : _urng{seed} {}

    [[nodiscard]] constexpr auto operator()(float v) noexcept -> float
    {
        auto const r      = _dist(_urng);
        auto const result = v + r - _last;
        _last             = r;
        return result;
    }

private:
    URNG _urng;
    etl::uniform_real_distribution<float> _dist{-0.5F, 0.5F};
    seed_type _last{0};
};

}  // namespace grit
