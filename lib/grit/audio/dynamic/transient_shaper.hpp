#pragma once

#include <grit/audio/envelope/envelope_follower.hpp>
#include <grit/unit/decibel.hpp>

#include <etl/concepts.hpp>

namespace grit {

template<etl::floating_point Float>
struct TransientShaper
{
    struct Parameter
    {
        Float attack{0};   // -1 to +1
        Float sustain{0};  // -1 to +1
    };

    TransientShaper() = default;

    auto setParameter(Parameter const& parameter) -> void;

    auto reset() -> void;
    auto prepare(Float sampleRate) -> void;
    [[nodiscard]] auto operator()(Float x) -> Float;

private:
    static constexpr auto const dbOffset = Float(1e-6);
    static constexpr auto const maxGain  = Float(32);

    Parameter _parameter{};
    Float _sampleRate{};

    EnvelopeFollower<Float> _attack1;
    EnvelopeFollower<Float> _attack2;

    EnvelopeFollower<Float> _sustain1;
    EnvelopeFollower<Float> _sustain2;
};

template<etl::floating_point Float>
auto TransientShaper<Float>::setParameter(Parameter const& parameter) -> void
{
    _parameter = parameter;

    _attack1.setParameter({Milliseconds<Float>{1}, Milliseconds<Float>{1'000}});
    _attack2.setParameter({Milliseconds<Float>{50}, Milliseconds<Float>{1'000}});

    auto const maxSustain = Float(1'000);
    auto const sustain    = maxSustain * parameter.sustain;
    _sustain1.setParameter({Milliseconds<Float>{1}, Milliseconds<Float>{sustain}});
    _sustain2.setParameter({Milliseconds<Float>{1}, Milliseconds<Float>{sustain / Float(20)}});
}

template<etl::floating_point Float>
auto TransientShaper<Float>::prepare(Float sampleRate) -> void
{
    _sampleRate = sampleRate;

    _attack1.prepare(sampleRate);
    _attack2.prepare(sampleRate);
    _sustain1.prepare(sampleRate);
    _sustain2.prepare(sampleRate);

    reset();
}

template<etl::floating_point Float>
auto TransientShaper<Float>::operator()(Float x) -> Float
{
    auto const absX = etl::abs(x);

    // Attack
    auto const aenv1 = toDecibels(_attack1(absX) + dbOffset);
    auto const aenv2 = toDecibels(_attack2(absX) + dbOffset);
    auto const adiff = etl::clamp((aenv1 - aenv2) * _parameter.attack, -maxGain, +maxGain);
    auto const again = fromDecibels(adiff);

    // Sustain
    auto const senv1 = toDecibels(_sustain1(absX) + dbOffset);
    auto const senv2 = toDecibels(_sustain2(absX) + dbOffset);
    auto const sdiff = etl::clamp((senv1 - senv2) * _parameter.sustain, -maxGain, +maxGain);
    auto const sgain = fromDecibels(sdiff);

    return x * (again * sgain);
}

template<etl::floating_point Float>
auto TransientShaper<Float>::reset() -> void
{
    _attack1.reset();
    _attack2.reset();
    _sustain1.reset();
    _sustain2.reset();
}

}  // namespace grit
