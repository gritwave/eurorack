#pragma once

#include <etl/cmath.hpp>
#include <etl/concepts.hpp>
#include <etl/numbers.hpp>

namespace grit {

enum struct cross_fade_curve
{
    Linear,
    ConstantPower,
    Logarithmic,
    Exponentail,
};

template<etl::floating_point SampleType>
struct cross_fade
{
    struct parameter
    {
        SampleType mix{0.5};
        cross_fade_curve curve{cross_fade_curve::Linear};
    };

    cross_fade() = default;

    auto set_parameter(parameter parameter) -> void;
    [[nodiscard]] auto get_parameter() const -> parameter;

    auto process(SampleType left, SampleType right) -> SampleType;

private:
    auto update() -> void;

    parameter _parameter{};
    SampleType _gain_l{0.5};
    SampleType _gain_r{0.5};
};

template<etl::floating_point SampleType>
auto cross_fade<SampleType>::set_parameter(parameter parameter) -> void
{
    _parameter = parameter;
    update();
}

template<etl::floating_point SampleType>
auto cross_fade<SampleType>::get_parameter() const -> parameter
{
    return _parameter;
}

template<etl::floating_point SampleType>
auto cross_fade<SampleType>::process(SampleType left, SampleType right) -> SampleType
{
    return (left * _gain_l) + (right * _gain_r);
}

template<etl::floating_point SampleType>
auto cross_fade<SampleType>::update() -> void
{
    static constexpr auto const log_min = static_cast<SampleType>(etl::log(0.000001));
    static constexpr auto const log_max = static_cast<SampleType>(etl::log(1.0));
    static constexpr auto const half_pi = static_cast<SampleType>(etl::numbers::pi * 0.5);

    switch (_parameter.curve) {
        case cross_fade_curve::Linear: {
            _gain_r = _parameter.mix;
            _gain_l = SampleType{1} - _gain_r;
            return;
        }
        case cross_fade_curve::ConstantPower: {
            _gain_r = etl::sin(_parameter.mix * half_pi);
            _gain_l = etl::sin((SampleType{1} - _parameter.mix) * half_pi);
            return;
        }
        case cross_fade_curve::Logarithmic: {
            _gain_r = etl::exp(_parameter.mix * (log_max - log_min) + log_min);
            _gain_l = SampleType{1} - _gain_r;
            return;
        }
        case cross_fade_curve::Exponentail: {
            _gain_r = _parameter.mix * _parameter.mix;
            _gain_l = SampleType{1} - _gain_r;
            return;
        }
        default: {
            _gain_r = SampleType{0.5};
            _gain_l = SampleType{0.5};
            return;
        }
    }
}

}  // namespace grit
