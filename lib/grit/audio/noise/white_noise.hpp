#pragma once

#include <etl/concepts.hpp>
#include <etl/random.hpp>

namespace grit {

/// \ingroup grit-audio-noise
template<etl::floating_point Float, typename URNG = etl::xoshiro128plusplus>
struct WhiteNoise
{
    using SeedType = typename URNG::result_type;

    WhiteNoise() = default;
    explicit WhiteNoise(SeedType seed);

    [[nodiscard]] auto operator()() -> Float;

private:
    URNG _rng{};
};

template<etl::floating_point Float, typename URNG>
WhiteNoise<Float, URNG>::WhiteNoise(SeedType seed) : _rng{seed}
{}

template<etl::floating_point Float, typename URNG>
auto WhiteNoise<Float, URNG>::operator()() -> Float
{
    auto dist = etl::uniform_real_distribution<Float>{Float(-1), Float(1)};
    return dist(_rng);
}

}  // namespace grit
