#pragma once

#include <etl/concepts.hpp>
#include <etl/random.hpp>

namespace mc
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
    SampleType _gain{};
    URNG _rng{};
    etl::uniform_real_distribution<SampleType> _dist{SampleType(-1), SampleType(1)};
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
    return _dist(_rng) * _gain;
}

}  // namespace mc
