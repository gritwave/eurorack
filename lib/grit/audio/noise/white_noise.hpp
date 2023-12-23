#pragma once

#include <etl/concepts.hpp>
#include <etl/random.hpp>

namespace grit {

template<etl::floating_point SampleType, typename URNG = etl::xoshiro128plusplus>
struct white_noise
{
    using SeedType = typename URNG::result_type;

    white_noise() = default;
    explicit white_noise(SeedType seed) noexcept;

    auto set_gain(SampleType gain) noexcept -> void;
    [[nodiscard]] auto get_gain() const noexcept -> SampleType;

    [[nodiscard]] auto process_sample() noexcept -> SampleType;

private:
    URNG _rng{};
    SampleType _gain{0.5};
};

template<etl::floating_point SampleType, typename URNG>
white_noise<SampleType, URNG>::white_noise(SeedType seed) noexcept : _rng{seed}
{}

template<etl::floating_point SampleType, typename URNG>
auto white_noise<SampleType, URNG>::set_gain(SampleType gain) noexcept -> void
{
    _gain = gain;
}

template<etl::floating_point SampleType, typename URNG>
auto white_noise<SampleType, URNG>::get_gain() const noexcept -> SampleType
{
    return _gain;
}

template<etl::floating_point SampleType, typename URNG>
auto white_noise<SampleType, URNG>::process_sample() noexcept -> SampleType
{
    auto dist = etl::uniform_real_distribution<SampleType>{SampleType(-1), SampleType(1)};
    return dist(_rng) * _gain;
}

}  // namespace grit
