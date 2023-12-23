#pragma once

#include <grit/audio/unit/time.hpp>

#include <etl/algorithm.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>

namespace grit {

template<etl::floating_point SampleType>
struct envelope_follower
{
    struct parameter
    {
        milliseconds<SampleType> attack{50};
        milliseconds<SampleType> release{50};
    };

    envelope_follower() = default;

    auto set_parameter(parameter const& parameter) noexcept -> void;

    auto reset() noexcept -> void;
    auto prepare(SampleType sample_rate) noexcept -> void;
    [[nodiscard]] auto process_sample(SampleType in) noexcept -> SampleType;

private:
    auto update() noexcept -> void;

    parameter _parameter{};
    SampleType _sample_rate{};
    SampleType _attack_coef{};
    SampleType _release_coef{};
    SampleType _envelope{};
};

template<etl::floating_point SampleType>
auto envelope_follower<SampleType>::set_parameter(parameter const& parameter) noexcept -> void
{
    _parameter = parameter;
    update();
}

template<etl::floating_point SampleType>
auto envelope_follower<SampleType>::prepare(SampleType sample_rate) noexcept -> void
{
    _sample_rate = sample_rate;
    update();
    reset();
}

template<etl::floating_point SampleType>
auto envelope_follower<SampleType>::process_sample(SampleType in) noexcept -> SampleType
{
    auto const env  = etl::abs(in);
    auto const coef = env > _envelope ? _attack_coef : _release_coef;

    _envelope = coef * (_envelope - env) + env;
    return _envelope;
}

template<etl::floating_point SampleType>
auto envelope_follower<SampleType>::reset() noexcept -> void
{
    _envelope = SampleType(0);
}

template<etl::floating_point SampleType>
auto envelope_follower<SampleType>::update() noexcept -> void
{
    static constexpr auto const log001 = etl::log(SampleType(0.01));

    auto const attack  = _parameter.attack.count();
    auto const release = _parameter.release.count();

    _attack_coef  = etl::exp(log001 / (attack * _sample_rate * SampleType(0.001)));
    _release_coef = etl::exp(log001 / (release * _sample_rate * SampleType(0.001)));
}

}  // namespace grit
