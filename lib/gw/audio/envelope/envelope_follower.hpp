#pragma once

#include <gw/audio/unit/time.hpp>

#include <etl/algorithm.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>

namespace gw {

template<etl::floating_point SampleType>
struct EnvelopeFollower
{
    struct Parameter
    {
        Milliseconds<SampleType> attack{50};
        Milliseconds<SampleType> release{50};
    };

    EnvelopeFollower() = default;

    auto setParameter(Parameter const& parameter) noexcept -> void;

    auto reset() noexcept -> void;
    auto prepare(SampleType sampleRate) noexcept -> void;
    [[nodiscard]] auto processSample(SampleType in) noexcept -> SampleType;

private:
    auto update() noexcept -> void;

    Parameter _parameter{};
    SampleType _sampleRate{};
    SampleType _attackCoef{};
    SampleType _releaseCoef{};
    SampleType _envelope{};
};

template<etl::floating_point SampleType>
auto EnvelopeFollower<SampleType>::setParameter(Parameter const& parameter) noexcept -> void
{
    _parameter = parameter;
    update();
}

template<etl::floating_point SampleType>
auto EnvelopeFollower<SampleType>::prepare(SampleType sampleRate) noexcept -> void
{
    _sampleRate = sampleRate;
    update();
    reset();
}

template<etl::floating_point SampleType>
auto EnvelopeFollower<SampleType>::processSample(SampleType in) noexcept -> SampleType
{
    auto const env  = etl::abs(in);
    auto const coef = env > _envelope ? _attackCoef : _releaseCoef;

    _envelope = coef * (_envelope - env) + env;
    return _envelope;
}

template<etl::floating_point SampleType>
auto EnvelopeFollower<SampleType>::reset() noexcept -> void
{
    _envelope = SampleType(0);
}

template<etl::floating_point SampleType>
auto EnvelopeFollower<SampleType>::update() noexcept -> void
{
    static constexpr auto const log001 = etl::log(SampleType(0.01));

    auto const attack  = _parameter.attack.count();
    auto const release = _parameter.release.count();

    _attackCoef  = etl::exp(log001 / (attack * _sampleRate * SampleType(0.001)));
    _releaseCoef = etl::exp(log001 / (release * _sampleRate * SampleType(0.001)));
}

}  // namespace gw
