#pragma once

#include <etl/concepts.hpp>
#include <etl/random.hpp>

namespace grit {

template<etl::floating_point Float, typename URNG = etl::xoshiro128plusplus>
struct white_noise
{
    using SeedType = typename URNG::result_type;

    white_noise() = default;
    explicit white_noise(SeedType seed) noexcept;

    auto set_gain(Float gain) noexcept -> void;
    [[nodiscard]] auto get_gain() const noexcept -> Float;

    [[nodiscard]] auto process_sample() noexcept -> Float;

private:
    URNG _rng{};
    Float _gain{0.5};
};

template<etl::floating_point Float, typename URNG>
white_noise<Float, URNG>::white_noise(SeedType seed) noexcept : _rng{seed}
{}

template<etl::floating_point Float, typename URNG>
auto white_noise<Float, URNG>::set_gain(Float gain) noexcept -> void
{
    _gain = gain;
}

template<etl::floating_point Float, typename URNG>
auto white_noise<Float, URNG>::get_gain() const noexcept -> Float
{
    return _gain;
}

template<etl::floating_point Float, typename URNG>
auto white_noise<Float, URNG>::process_sample() noexcept -> Float
{
    auto dist = etl::uniform_real_distribution<Float>{Float(-1), Float(1)};
    return dist(_rng) * _gain;
}

}  // namespace grit
