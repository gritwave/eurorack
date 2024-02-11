#pragma once

#include <etl/random.hpp>

namespace grit {

/// \brief Dummy dithering class. Performs no dithering.
/// \ingroup grit-audio-noise
template<typename URNG>
struct NoDither
{
    using seed_type = typename URNG::result_type;

    explicit constexpr NoDither(seed_type /*unused*/) {}

    [[nodiscard]] constexpr auto operator()(float v) -> float { return v; }
};

/// \ingroup grit-audio-noise
template<typename URNG>
struct RectangleDither
{
    using seed_type = typename URNG::result_type;

    explicit constexpr RectangleDither(seed_type seed) : _urng{seed} {}

    [[nodiscard]] constexpr auto operator()(float v) -> float { return v + _dist(_urng); }

private:
    URNG _urng;
    etl::uniform_real_distribution<float> _dist{-0.5F, 0.5F};
};

/// \ingroup grit-audio-noise
template<typename URNG>
struct TriangleDither
{
    using seed_type = typename URNG::result_type;

    explicit constexpr TriangleDither(seed_type seed) : _urng{seed} {}

    [[nodiscard]] constexpr auto operator()(float v) -> float
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
