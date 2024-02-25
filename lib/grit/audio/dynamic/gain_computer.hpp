#pragma once

#include <grit/unit/decibel.hpp>

namespace grit {

/// \ingroup grit-audio-dynamic
template<etl::floating_point Float>
struct GainComputerParameter
{
    Decibels<Float> threshold{0.0};
    Decibels<Float> knee{0.0};
    Float ratio{1.0};
};

/// \ingroup grit-audio-dynamic
template<etl::floating_point Float>
struct HardKneeGainComputer
{
    using SampleType = Float;
    using Parameter  = GainComputerParameter<Float>;

    HardKneeGainComputer() = default;

    auto setParameter(Parameter parameter) -> void
    {
        _threshold = parameter.threshold;
        _ratio     = parameter.ratio;
    }

    [[nodiscard]] auto operator()(Float xg) -> Float
    {
        auto const t = _threshold.value();
        auto const r = _ratio;
        return xg < t ? xg : t + (xg - t) / r;
    }

private:
    Decibels<Float> _threshold{0};
    Float _ratio{1};
};

/// \ingroup grit-audio-dynamic
template<etl::floating_point Float>
struct SoftKneeGainComputer
{
    using SampleType = Float;
    using Parameter  = GainComputerParameter<Float>;

    SoftKneeGainComputer() = default;

    auto setParameter(Parameter parameter) -> void { _parameter = parameter; }

    [[nodiscard]] auto operator()(Float x) -> Float
    {
        auto const t = _parameter.threshold.value();
        auto const w = _parameter.knee.value();
        auto const r = _parameter.ratio;

        if (w < Float(2) * t - Float(2) * x) {
            return x;
        }
        if (w > Float(2) * etl::abs(t - x)) {
            auto const tmp = -t + Float(0.5) * w + x;
            return x + Float(0.5) * (Float(-1.0F) + Float(1.0F) / r) * (tmp * tmp) / w;
        }

        return t + (-t + x) / r;
    }

private:
    Parameter _parameter{};
};

}  // namespace grit
