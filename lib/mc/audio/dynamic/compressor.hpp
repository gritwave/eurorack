#pragma once

#include <mc/audio/unit/time.hpp>

#include <etl/algorithm.hpp>
#include <etl/concepts.hpp>
#include <etl/numbers.hpp>

#include <cmath>

namespace mc::audio
{

template<etl::floating_point SampleType>
struct Compressor
{
    struct Parameter
    {
        SampleType threshold{0};
        SampleType ratio{1};
        SampleType knee{0};

        Milliseconds<float> attack{0};
        Milliseconds<float> release{0};

        SampleType makeUp{0};
        SampleType wet{0};
    };

    Compressor() = default;

    auto setParameter(Parameter const& parameter) -> void { _parameter = parameter; }

    [[nodiscard]] auto getGainReduction() const -> SampleType;

    auto prepare(SampleType sampleRate) -> void
    {
        _sampleRate = sampleRate;
        reset();
    }

    [[nodiscard]] auto processSample(SampleType signal, SampleType sideChain) noexcept -> SampleType
    {
        auto const threshold = _parameter.threshold;
        auto const ratio     = _parameter.ratio > SampleType(30) ? SampleType(200) : _parameter.ratio;
        auto const knee      = _parameter.knee;
        auto const alphaA    = calculateAttackOrRelease(_parameter.attack);
        auto const alphaR    = calculateAttackOrRelease(_parameter.release);

        auto const inputSquared = sideChain * sideChain;
        auto const env          = (inputSquared <= static_cast<SampleType>(1e-6)) ? -SampleType(60)
                                                                                  : SampleType(10) * std::log10(inputSquared);

        auto const halfKneeRange  = -(knee * (-SampleType(60) - threshold) / SampleType(4));
        auto const fullKneeRange  = halfKneeRange + halfKneeRange / ratio;
        auto const kneedThreshold = threshold - halfKneeRange;
        auto const ceilThreshold  = threshold + halfKneeRange / ratio;
        auto const limit          = etl::clamp(env, kneedThreshold, ceilThreshold);
        auto const factor         = -((limit - kneedThreshold) / fullKneeRange) + SampleType(1);
        auto const ratioQuotient
            = knee > SampleType(0) ? ratio * factor + SampleType(1) * (-factor + SampleType(1)) : SampleType(1);

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

    auto reset() -> void { _ylPrev = SampleType(0); }

private:
    [[nodiscard]] auto calculateAttackOrRelease(Seconds<SampleType> value) const noexcept -> SampleType
    {
        static constexpr auto const euler = static_cast<SampleType>(etl::numbers::e);

        auto const sec = value.count();
        if (sec == SampleType(0)) { return SampleType(0); }
        return std::pow(SampleType(1) / euler, SampleType(1) / static_cast<SampleType>(_sampleRate) / sec);
    }

    Parameter _parameter{};
    SampleType _sampleRate{};
    SampleType _ylPrev{};
    SampleType _reduction{1};
};

}  // namespace mc::audio
