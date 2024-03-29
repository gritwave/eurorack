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
        Decibels<Float> knee{0.0};
        Float ratio{1.0};

        Milliseconds<Float> attack{50};
        Milliseconds<Float> release{50};
    };

    Dynamic() = default;

    auto setParameter(Parameter param) -> void
    {
        _gainComputer.setParameter({param.threshold, param.knee, param.ratio});
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
    TETL_NO_UNIQUE_ADDRESS LevelDetector _levelDetector;
    TETL_NO_UNIQUE_ADDRESS GainComputer _gainComputer;
    TETL_NO_UNIQUE_ADDRESS Ballistics _ballistics;
};

}  // namespace grit
