#pragma once

#include <grit/unit/time.hpp>

#include <etl/algorithm.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>
#include <etl/numbers.hpp>
#include <etl/optional.hpp>

#include <cmath>

namespace grit {

template<etl::floating_point Float>
struct Compressor
{
    struct Parameter
    {
        Float threshold{0};
        Float ratio{1};
        Float knee{0};

        Milliseconds<Float> attack{0};
        Milliseconds<Float> release{0};

        Float makeUp{0};
        Float wet{0};
    };

    Compressor() = default;

    auto setParameter(Parameter const& parameter) -> void;

    auto reset() -> void;
    auto prepare(Float sampleRate) -> void;
    [[nodiscard]] auto operator()(Float signal, Float sideChain) -> Float;

    [[nodiscard]] auto getGainReduction() const -> Float;

private:
    [[nodiscard]] auto calculateTimeAlpha(Seconds<Float> value) const -> Float;

    Parameter _parameter{};
    Float _sampleRate{};
    Float _ylPrev{};
    Float _reduction{1};
};

template<etl::floating_point Float>
auto Compressor<Float>::setParameter(Parameter const& parameter) -> void
{
    _parameter = parameter;
}

template<etl::floating_point Float>
auto Compressor<Float>::prepare(Float sampleRate) -> void
{
    _sampleRate = sampleRate;
    reset();
}

template<etl::floating_point Float>
auto Compressor<Float>::operator()(Float signal, Float sideChain) -> Float
{
    auto const threshold = _parameter.threshold;
    auto const ratio     = _parameter.ratio;
    auto const knee      = _parameter.knee;
    auto const alphaA    = calculateTimeAlpha(_parameter.attack);
    auto const alphaR    = calculateTimeAlpha(_parameter.release);

    auto const in  = sideChain * sideChain;
    auto const env = in <= static_cast<Float>(1e-6) ? -Float(60) : Float(10) * std::log10(in);

    auto const halfKneeRange  = -(knee * (-Float(60) - threshold) / Float(4));
    auto const fullKneeRange  = halfKneeRange + halfKneeRange / ratio;
    auto const kneedThreshold = threshold - halfKneeRange;
    auto const ceilThreshold  = threshold + halfKneeRange / ratio;
    auto const limit          = etl::clamp(env, kneedThreshold, ceilThreshold);
    auto const factor         = -((limit - kneedThreshold) / fullKneeRange) + Float(1);
    auto const ratioQuotient  = knee > 0 ? ratio * factor + Float(1) * (-factor + Float(1)) : Float(1);

    auto yg = Float(0);
    if (env < kneedThreshold) {
        yg = env;
    } else {
        yg = kneedThreshold + (env - kneedThreshold) / (ratio / ratioQuotient);
    }

    auto yl       = Float(0);
    auto const xl = env - yg;
    if (xl > _ylPrev) {
        yl = alphaA * _ylPrev + (Float(1) - alphaA) * xl;
    } else {
        yl = alphaR * _ylPrev + (Float(1) - alphaR) * xl;
    }

    auto const controlCompressor = etl::pow(Float(10), (Float(1) - yl) * Float(0.05));

    _reduction = controlCompressor;
    _ylPrev    = yl;

    auto const wet    = _parameter.wet;
    auto const makeup = _parameter.makeUp;

    auto const wetSample = signal * controlCompressor * makeup;
    auto const drySample = signal;

    return wetSample * wet + drySample * (1.0F - wet);
}

template<etl::floating_point Float>
auto Compressor<Float>::getGainReduction() const -> Float
{
    return _reduction;
}

template<etl::floating_point Float>
auto Compressor<Float>::reset() -> void
{
    _ylPrev = Float(0);
}

template<etl::floating_point Float>
auto Compressor<Float>::calculateTimeAlpha(Seconds<Float> value) const -> Float
{
    static constexpr auto const euler = static_cast<Float>(etl::numbers::e);

    auto const sec = value.count();
    if (sec == Float(0)) {
        return Float(0);
    }
    return etl::pow(Float(1) / euler, Float(1) / static_cast<Float>(_sampleRate) / sec);
}

}  // namespace grit
