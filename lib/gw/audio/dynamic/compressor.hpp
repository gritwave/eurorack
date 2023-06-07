#pragma once

#include <gw/audio/unit/time.hpp>

#include <etl/algorithm.hpp>
#include <etl/concepts.hpp>
#include <etl/numbers.hpp>
#include <etl/optional.hpp>

#include <cmath>

namespace gw
{

template<etl::floating_point SampleType>
struct Compressor
{
    struct Parameter
    {
        SampleType threshold{0};
        SampleType ratio{1};
        SampleType knee{0};

        Milliseconds<SampleType> attack{0};
        Milliseconds<SampleType> release{0};

        SampleType makeUp{0};
        SampleType wet{0};
    };

    Compressor() = default;

    auto setParameter(Parameter const& parameter) noexcept -> void;

    auto reset() noexcept -> void;
    auto prepare(SampleType sampleRate) noexcept -> void;
    [[nodiscard]] auto processSample(SampleType signal, SampleType sideChain) noexcept -> SampleType;

    [[nodiscard]] auto getGainReduction() const noexcept -> SampleType;

private:
    [[nodiscard]] auto calculateTimeAlpha(Seconds<SampleType> value) const noexcept -> SampleType;

    Parameter _parameter{};
    SampleType _sampleRate{};
    SampleType _ylPrev{};
    SampleType _reduction{1};
};

template<etl::floating_point SampleType>
auto Compressor<SampleType>::setParameter(Parameter const& parameter) noexcept -> void
{
    _parameter = parameter;
}

template<etl::floating_point SampleType>
auto Compressor<SampleType>::prepare(SampleType sampleRate) noexcept -> void
{
    _sampleRate = sampleRate;
    reset();
}

template<etl::floating_point SampleType>
auto Compressor<SampleType>::processSample(SampleType signal, SampleType sideChain) noexcept -> SampleType
{
    auto const threshold = _parameter.threshold;
    auto const ratio     = _parameter.ratio;
    auto const knee      = _parameter.knee;
    auto const alphaA    = calculateTimeAlpha(_parameter.attack);
    auto const alphaR    = calculateTimeAlpha(_parameter.release);

    auto const in  = sideChain * sideChain;
    auto const env = in <= static_cast<SampleType>(1e-6) ? -SampleType(60) : SampleType(10) * std::log10(in);

    auto const halfKneeRange  = -(knee * (-SampleType(60) - threshold) / SampleType(4));
    auto const fullKneeRange  = halfKneeRange + halfKneeRange / ratio;
    auto const kneedThreshold = threshold - halfKneeRange;
    auto const ceilThreshold  = threshold + halfKneeRange / ratio;
    auto const limit          = etl::clamp(env, kneedThreshold, ceilThreshold);
    auto const factor         = -((limit - kneedThreshold) / fullKneeRange) + SampleType(1);
    auto const ratioQuotient  = knee > 0 ? ratio * factor + SampleType(1) * (-factor + SampleType(1)) : SampleType(1);

    auto yg = SampleType(0);
    if (env < kneedThreshold) { yg = env; }
    else { yg = kneedThreshold + (env - kneedThreshold) / (ratio / ratioQuotient); }

    auto yl       = SampleType(0);
    auto const xl = env - yg;
    if (xl > _ylPrev) { yl = alphaA * _ylPrev + (SampleType(1) - alphaA) * xl; }
    else { yl = alphaR * _ylPrev + (SampleType(1) - alphaR) * xl; }

    auto const controlCompressor = std::pow(SampleType(10), (SampleType(1) - yl) * SampleType(0.05));

    _reduction = controlCompressor;
    _ylPrev    = yl;

    auto const wet    = _parameter.wet;
    auto const makeup = _parameter.makeUp;

    auto const wetSample = signal * controlCompressor * makeup;
    auto const drySample = signal;

    return wetSample * wet + drySample * (1.0F - wet);
}

template<etl::floating_point SampleType>
auto Compressor<SampleType>::getGainReduction() const noexcept -> SampleType
{
    return _reduction;
}

template<etl::floating_point SampleType>
auto Compressor<SampleType>::reset() noexcept -> void
{
    _ylPrev = SampleType(0);
}

template<etl::floating_point SampleType>
auto Compressor<SampleType>::calculateTimeAlpha(Seconds<SampleType> value) const noexcept -> SampleType
{
    static constexpr auto const euler = static_cast<SampleType>(etl::numbers::e);

    auto const sec = value.count();
    if (sec == SampleType(0)) { return SampleType(0); }
    return std::pow(SampleType(1) / euler, SampleType(1) / static_cast<SampleType>(_sampleRate) / sec);
}

}  // namespace gw
