#pragma once

#include <etl/concepts.hpp>
#include <etl/random.hpp>

namespace gw
{

template<etl::floating_point SampleType, typename URNG = etl::xoshiro128plusplus>
struct WhiteNoise
{
    using SeedType = typename URNG::result_type;

    WhiteNoise() = default;
    explicit WhiteNoise(SeedType seed) noexcept;

    auto setGain(SampleType gain) noexcept -> void;
    [[nodiscard]] auto getGain() const noexcept -> SampleType;

    [[nodiscard]] auto processSample() noexcept -> SampleType;

private:
    URNG _rng{};
    SampleType _gain{0.5};
};

template<etl::floating_point SampleType, typename URNG>
WhiteNoise<SampleType, URNG>::WhiteNoise(SeedType seed) noexcept : _rng{seed}
{
}

template<etl::floating_point SampleType, typename URNG>
auto WhiteNoise<SampleType, URNG>::setGain(SampleType gain) noexcept -> void
{
    _gain = gain;
}

template<etl::floating_point SampleType, typename URNG>
auto WhiteNoise<SampleType, URNG>::getGain() const noexcept -> SampleType
{
    return _gain;
}

template<etl::floating_point SampleType, typename URNG>
auto WhiteNoise<SampleType, URNG>::processSample() noexcept -> SampleType
{
    auto dist = etl::uniform_real_distribution<SampleType>{SampleType(-1), SampleType(1)};
    return dist(_rng) * _gain;
}

}  // namespace gw
