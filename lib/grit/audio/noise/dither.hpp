#pragma once

#include <etl/random.hpp>

namespace grit {

/// \brief Dummy dithering class. Performs no dithering.
/// \ingroup grit-audio-noise
template<typename URNG>
struct NoDither
{
    using SeedType = typename URNG::result_type;

    explicit constexpr NoDither(SeedType /*unused*/) {}

    [[nodiscard]] constexpr auto operator()(float v) -> float { return v; }
};

/// \ingroup grit-audio-noise
template<typename URNG>
struct RectangleDither
{
    using SeedType = typename URNG::result_type;

    explicit constexpr RectangleDither(SeedType seed) : _urng{seed} {}

    [[nodiscard]] constexpr auto operator()(float v) -> float { return v + _dist(_urng); }

private:
    URNG _urng;
    etl::uniform_real_distribution<float> _dist{-0.5F, 0.5F};
};

/// \ingroup grit-audio-noise
template<typename URNG>
struct TriangleDither
{
    using SeedType = typename URNG::result_type;

    explicit constexpr TriangleDither(SeedType seed) : _urng{seed} {}

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
    SeedType _last{0};
};

}  // namespace grit
