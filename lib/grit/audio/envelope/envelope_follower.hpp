#pragma once

#include <grit/audio/unit/time.hpp>

#include <etl/algorithm.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>

namespace grit {

template<etl::floating_point Float>
struct envelope_follower
{
    struct parameter
    {
        milliseconds<Float> attack{50};
        milliseconds<Float> release{50};
    };

    envelope_follower() = default;

    auto set_parameter(parameter const& parameter) noexcept -> void;

    auto reset() noexcept -> void;
    auto prepare(Float sample_rate) noexcept -> void;
    [[nodiscard]] auto process_sample(Float in) noexcept -> Float;

private:
    auto update() noexcept -> void;

    parameter _parameter{};
    Float _sample_rate{};
    Float _attack_coef{};
    Float _release_coef{};
    Float _envelope{};
};

template<etl::floating_point Float>
auto envelope_follower<Float>::set_parameter(parameter const& parameter) noexcept -> void
{
    _parameter = parameter;
    update();
}

template<etl::floating_point Float>
auto envelope_follower<Float>::prepare(Float sample_rate) noexcept -> void
{
    _sample_rate = sample_rate;
    update();
    reset();
}

template<etl::floating_point Float>
auto envelope_follower<Float>::process_sample(Float in) noexcept -> Float
{
    auto const env  = etl::abs(in);
    auto const coef = env > _envelope ? _attack_coef : _release_coef;

    _envelope = coef * (_envelope - env) + env;
    return _envelope;
}

template<etl::floating_point Float>
auto envelope_follower<Float>::reset() noexcept -> void
{
    _envelope = Float(0);
}

template<etl::floating_point Float>
auto envelope_follower<Float>::update() noexcept -> void
{
    static constexpr auto const log001 = etl::log(Float(0.01));

    auto const attack  = _parameter.attack.count();
    auto const release = _parameter.release.count();

    _attack_coef  = etl::exp(log001 / (attack * _sample_rate * Float(0.001)));
    _release_coef = etl::exp(log001 / (release * _sample_rate * Float(0.001)));
}

}  // namespace grit
