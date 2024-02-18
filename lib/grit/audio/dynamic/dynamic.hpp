#pragma once

#include <grit/unit/decibel.hpp>
#include <grit/unit/time.hpp>

namespace grit {

/// \ingroup grit-audio-dynamic
template<etl::floating_point Float, typename LevelDetector, typename GainComputer, typename Ballistics>
struct Dynamic
{
    struct Parameter
    {
        Decibels<Float> threshold{0.0};
        Decibels<Float> ratio{1.0};
        Decibels<Float> knee{0.0};
        Milliseconds<Float> attack{50};
        Milliseconds<Float> release{50};
    };

    Dynamic() = default;

    auto setParameter(Parameter param) -> void
    {
        _gainComputer.setParameter({param.threshold, param.ratio, param.knee});
        _ballistics.setParameter({param.attack, param.release});
    }

    auto setSampleRate(Float sampleRate) -> void { _ballistics.setSampleRate(sampleRate); }

    [[nodiscard]] auto operator()(Float x) -> Float { return (*this)(x, x); }

    [[nodiscard]] auto operator()(Float x, Float sidechain) -> Float
    {
        static constexpr auto const makeUpGain = Float(0);

        auto const xg = _levelDetector(sidechain);
        auto const yg = _gainComputer(xg);
        auto const xl = xg - yg;
        auto const yl = _ballistics(xl);
        return x * fromDecibels(makeUpGain - yl);
    }

private:
    [[no_unique_address]] LevelDetector _levelDetector;
    [[no_unique_address]] GainComputer _gainComputer;
    [[no_unique_address]] Ballistics _ballistics;
};

}  // namespace grit
