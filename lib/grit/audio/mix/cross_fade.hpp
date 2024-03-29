#pragma once

#include <etl/cmath.hpp>
#include <etl/concepts.hpp>
#include <etl/numbers.hpp>

namespace grit {

/// \ingroup grit-audio-mix
enum struct CrossFadeCurve
{
    Linear,
    ConstantPower,
    Logarithmic,
    Exponentail,
};

/// \ingroup grit-audio-mix
template<etl::floating_point Float>
struct CrossFade
{
    using Curve = CrossFadeCurve;

    struct Parameter
    {
        Float mix{0.5};
        CrossFadeCurve curve{CrossFadeCurve::Linear};
    };

    CrossFade() = default;

    auto setParameter(Parameter parameter) -> void;
    [[nodiscard]] auto getParameter() const -> Parameter;

    auto operator()(Float left, Float right) -> Float;

private:
    auto update() -> void;

    Parameter _parameter{};
    Float _gainL{0.5};
    Float _gainR{0.5};
};

template<etl::floating_point Float>
auto CrossFade<Float>::setParameter(Parameter parameter) -> void
{
    _parameter = parameter;
    update();
}

template<etl::floating_point Float>
auto CrossFade<Float>::getParameter() const -> Parameter
{
    return _parameter;
}

template<etl::floating_point Float>
auto CrossFade<Float>::operator()(Float left, Float right) -> Float
{
    return (left * _gainL) + (right * _gainR);
}

template<etl::floating_point Float>
auto CrossFade<Float>::update() -> void
{
    static constexpr auto const logMin = static_cast<Float>(etl::log(0.000001));
    static constexpr auto const logMax = static_cast<Float>(etl::log(1.0));
    static constexpr auto const halfPi = static_cast<Float>(etl::numbers::pi * 0.5);

    switch (_parameter.curve) {
        case CrossFadeCurve::Linear: {
            _gainR = _parameter.mix;
            _gainL = Float{1} - _gainR;
            return;
        }
        case CrossFadeCurve::ConstantPower: {
            _gainR = etl::sin(_parameter.mix * halfPi);
            _gainL = etl::sin((Float{1} - _parameter.mix) * halfPi);
            return;
        }
        case CrossFadeCurve::Logarithmic: {
            _gainR = etl::exp(_parameter.mix * (logMax - logMin) + logMin);
            _gainL = Float{1} - _gainR;
            return;
        }
        case CrossFadeCurve::Exponentail: {
            _gainR = _parameter.mix * _parameter.mix;
            _gainL = Float{1} - _gainR;
            return;
        }
        default: {
            _gainR = Float{0.5};
            _gainL = Float{0.5};
            return;
        }
    }
}

}  // namespace grit
