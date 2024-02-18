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
    using value_type = Float;
    using Parameter  = GainComputerParameter<Float>;

    HardKneeGainComputer() = default;

    auto setParameter(Parameter parameter) -> void
    {
        _threshold = parameter.threshold;
        _ratio     = parameter.ratio;
    }

    [[nodiscard]] auto operator()(Float xg) -> Float
    {
        auto const T = _threshold.value();
        auto const R = _ratio;
        return xg < T ? xg : T + (xg - T) / R;
    }

private:
    Decibels<Float> _threshold{0};
    Float _ratio{1};
};

/// \ingroup grit-audio-dynamic
template<etl::floating_point Float>
struct SoftKneeGainComputer
{
    using value_type = Float;
    using Parameter  = GainComputerParameter<Float>;

    SoftKneeGainComputer() = default;

    auto setParameter(Parameter parameter) -> void { _parameter = parameter; }

    [[nodiscard]] auto operator()(Float x) -> Float
    {
        auto const T = _parameter.threshold.value();
        auto const W = _parameter.knee.value();
        auto const R = _parameter.ratio;

        if (W < Float(2) * T - Float(2) * x) {
            return x;
        } else if (W > Float(2) * etl::abs(T - x)) {
            auto const tmp = -T + Float(0.5) * W + x;
            return x + Float(0.5) * (Float(-1.0F) + Float(1.0F) / R) * (tmp * tmp) / W;
        }

        return T + (-T + x) / R;
    }

private:
    Parameter _parameter{};
};

}  // namespace grit
