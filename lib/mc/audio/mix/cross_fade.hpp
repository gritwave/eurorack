#pragma once

#include <etl/cmath.hpp>
#include <etl/concepts.hpp>
#include <etl/numbers.hpp>

#include <cmath>

namespace mc
{

enum struct CrossFadeCurve
{
    Linear,
    ConstantPower,
    Logarithmic,
    Exponentail,
};

template<etl::floating_point SampleType>
struct CrossFade
{
    struct Parameter
    {
        SampleType mix{0.5};
        CrossFadeCurve curve{CrossFadeCurve::Linear};
    };

    CrossFade() = default;

    auto setParameter(Parameter parameter) -> void;
    [[nodiscard]] auto getParameter() const -> Parameter;

    auto process(SampleType left, SampleType right) -> SampleType;

private:
    auto update() -> void;

    Parameter _parameter{};
    SampleType _gainL{0.5};
    SampleType _gainR{0.5};
};

template<etl::floating_point SampleType>
auto CrossFade<SampleType>::setParameter(Parameter parameter) -> void
{
    _parameter = parameter;
    update();
}

template<etl::floating_point SampleType>
auto CrossFade<SampleType>::getParameter() const -> Parameter
{
    return _parameter;
}

template<etl::floating_point SampleType>
auto CrossFade<SampleType>::process(SampleType left, SampleType right) -> SampleType
{
    return (left * _gainL) + (right * _gainR);
}

template<etl::floating_point SampleType>
auto CrossFade<SampleType>::update() -> void
{
    static constexpr auto const logMin = static_cast<SampleType>(etl::log(0.000001));
    static constexpr auto const logMax = static_cast<SampleType>(etl::log(1.0));
    static constexpr auto const halfPi = static_cast<SampleType>(etl::numbers::pi * 0.5);

    switch (_parameter.curve)
    {
        case CrossFadeCurve::Linear:
        {
            _gainR = _parameter.mix;
            _gainL = SampleType{1} - _gainR;
            return;
        }
        case CrossFadeCurve::ConstantPower:
        {
            _gainR = std::sin(_parameter.mix * halfPi);
            _gainL = std::sin((SampleType{1} - _parameter.mix) * halfPi);
            return;
        }
        case CrossFadeCurve::Logarithmic:
        {
            _gainR = std::exp(_parameter.mix * (logMax - logMin) + logMin);
            _gainL = SampleType{1} - _gainR;
            return;
        }
        case CrossFadeCurve::Exponentail:
        {
            _gainR = _parameter.mix * _parameter.mix;
            _gainL = SampleType{1} - _gainR;
            return;
        }
        default:
        {
            _gainR = SampleType{0.5};
            _gainL = SampleType{0.5};
            return;
        }
    }
}

}  // namespace mc
