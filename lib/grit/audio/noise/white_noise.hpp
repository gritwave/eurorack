#pragma once

#include <etl/concepts.hpp>
#include <etl/random.hpp>

namespace grit {

template<etl::floating_point Float, typename URNG = etl::xoshiro128plusplus>
struct WhiteNoise
{
    using SeedType = typename URNG::result_type;

    WhiteNoise() = default;
    explicit WhiteNoise(SeedType seed);

    auto setGain(Float gain) -> void;
    [[nodiscard]] auto getGain() const -> Float;

    [[nodiscard]] auto processSample() -> Float;

private:
    URNG _rng{};
    Float _gain{0.5};
};

template<etl::floating_point Float, typename URNG>
WhiteNoise<Float, URNG>::WhiteNoise(SeedType seed) : _rng{seed}
{}

template<etl::floating_point Float, typename URNG>
auto WhiteNoise<Float, URNG>::setGain(Float gain) -> void
{
    _gain = gain;
}

template<etl::floating_point Float, typename URNG>
auto WhiteNoise<Float, URNG>::getGain() const -> Float
{
    return _gain;
}

template<etl::floating_point Float, typename URNG>
auto WhiteNoise<Float, URNG>::processSample() -> Float
{
    auto dist = etl::uniform_real_distribution<Float>{Float(-1), Float(1)};
    return dist(_rng) * _gain;
}

}  // namespace grit
