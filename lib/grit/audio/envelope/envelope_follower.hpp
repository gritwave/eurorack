#pragma once

#include <grit/audio/unit/time.hpp>

#include <etl/algorithm.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>

namespace grit {

template<etl::floating_point Float>
struct EnvelopeFollower
{
    struct Parameter
    {
        milliseconds<Float> attack{50};
        milliseconds<Float> release{50};
    };

    EnvelopeFollower() = default;

    auto setParameter(Parameter const& parameter) noexcept -> void;

    auto reset() noexcept -> void;
    auto prepare(Float sampleRate) noexcept -> void;
    [[nodiscard]] auto processSample(Float in) noexcept -> Float;

private:
    auto update() noexcept -> void;

    Parameter _parameter{};
    Float _sampleRate{};
    Float _attackCoef{};
    Float _releaseCoef{};
    Float _envelope{};
};

template<etl::floating_point Float>
auto EnvelopeFollower<Float>::setParameter(Parameter const& parameter) noexcept -> void
{
    _parameter = parameter;
    update();
}

template<etl::floating_point Float>
auto EnvelopeFollower<Float>::prepare(Float sampleRate) noexcept -> void
{
    _sampleRate = sampleRate;
    update();
    reset();
}

template<etl::floating_point Float>
auto EnvelopeFollower<Float>::processSample(Float in) noexcept -> Float
{
    auto const env  = etl::abs(in);
    auto const coef = env > _envelope ? _attackCoef : _releaseCoef;

    _envelope = coef * (_envelope - env) + env;
    return _envelope;
}

template<etl::floating_point Float>
auto EnvelopeFollower<Float>::reset() noexcept -> void
{
    _envelope = Float(0);
}

template<etl::floating_point Float>
auto EnvelopeFollower<Float>::update() noexcept -> void
{
    static constexpr auto const log001 = etl::log(Float(0.01));

    auto const attack  = _parameter.attack.count();
    auto const release = _parameter.release.count();

    _attackCoef  = etl::exp(log001 / (attack * _sampleRate * Float(0.001)));
    _releaseCoef = etl::exp(log001 / (release * _sampleRate * Float(0.001)));
}

}  // namespace grit
