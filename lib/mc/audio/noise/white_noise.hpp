#pragma once

#include <etl/concepts.hpp>
#include <etl/random.hpp>

namespace mc
{

template<etl::floating_point SampleType>
struct WhiteNoise
{
    WhiteNoise() = default;
    explicit WhiteNoise(etl::uint32_t seed) noexcept;

    auto setGain(SampleType gain) noexcept -> void;
    [[nodiscard]] auto getGain() const noexcept -> SampleType;

    [[nodiscard]] auto processSample() noexcept -> SampleType;

private:
    SampleType _gain{};
    etl::xoshiro128plusplus _rng{45643213};
    etl::uniform_real_distribution<SampleType> _dist{SampleType(-1), SampleType(1)};
};

template<etl::floating_point SampleType>
WhiteNoise<SampleType>::WhiteNoise(etl::uint32_t seed) noexcept : _rng{seed}
{
}

template<etl::floating_point SampleType>
auto WhiteNoise<SampleType>::setGain(SampleType gain) noexcept -> void
{
    _gain = gain;
}

template<etl::floating_point SampleType>
auto WhiteNoise<SampleType>::getGain() const noexcept -> SampleType
{
    return _gain;
}

template<etl::floating_point SampleType>
auto WhiteNoise<SampleType>::processSample() noexcept -> SampleType
{
    return _dist(_rng) * _gain;
}

}  // namespace mc
