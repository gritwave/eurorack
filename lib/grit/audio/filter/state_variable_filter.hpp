#pragma once

#include <etl/cmath.hpp>
#include <etl/concepts.hpp>
#include <etl/numbers.hpp>
#include <etl/type_traits.hpp>

namespace grit {

/// \ingroup grit-audio-filter
enum struct StateVariableFilterType
{
    Highpass,
    Bandpass,
    Lowpass,
    Notch,
    Peak,
    Allpass,
};

/// \brief State variable filter
/// \details https://cytomic.com/files/dsp/SvfLinearTrapAllOutputs.pdf
/// \ingroup grit-audio-filter
template<etl::floating_point Float, StateVariableFilterType Type>
struct StateVariableFilter
{
    using SampleType = Float;

    struct Parameter
    {
        Float cutoff    = Float(440);
        Float resonance = Float(1) / etl::sqrt(Float(2));
    };

    StateVariableFilter() = default;

    auto setParameter(Parameter const& parameter) -> void;
    auto setSampleRate(Float sampleRate) -> void;
    auto operator()(Float x) -> Float;
    auto reset() -> void;

private:
    auto update() -> void;

    Parameter _parameter{};
    Float _sampleRate{0};

    Float _g{0};
    Float _k{0};
    Float _gt0{0};
    Float _gk0{0};

    Float _ic1eq{0};
    Float _ic2eq{0};
};

/// \ingroup grit-audio-filter
template<etl::floating_point Float>
using StateVariableHighpass = StateVariableFilter<Float, StateVariableFilterType::Highpass>;

/// \ingroup grit-audio-filter
template<etl::floating_point Float>
using StateVariableBandpass = StateVariableFilter<Float, StateVariableFilterType::Bandpass>;

/// \ingroup grit-audio-filter
template<etl::floating_point Float>
using StateVariableLowpass = StateVariableFilter<Float, StateVariableFilterType::Lowpass>;

/// \ingroup grit-audio-filter
template<etl::floating_point Float>
using StateVariableNotch = StateVariableFilter<Float, StateVariableFilterType::Notch>;

/// \ingroup grit-audio-filter
template<etl::floating_point Float>
using StateVariablePeak = StateVariableFilter<Float, StateVariableFilterType::Peak>;

/// \ingroup grit-audio-filter
template<etl::floating_point Float>
using StateVariableAllpass = StateVariableFilter<Float, StateVariableFilterType::Allpass>;

template<etl::floating_point Float, StateVariableFilterType Type>
auto StateVariableFilter<Float, Type>::setParameter(Parameter const& parameter) -> void
{
    _parameter = parameter;
    update();
}

template<etl::floating_point Float, StateVariableFilterType Type>
auto StateVariableFilter<Float, Type>::setSampleRate(Float sampleRate) -> void
{
    _sampleRate = sampleRate;
    update();
    reset();
}

template<etl::floating_point Float, StateVariableFilterType Type>
auto StateVariableFilter<Float, Type>::operator()(Float x) -> Float
{
    auto const t0 = x - _ic2eq;
    auto const v0 = _gt0 * t0 - _gk0 * _ic1eq;
    auto const t1 = _g * v0;
    auto const v1 = _ic1eq + t1;
    auto const t2 = _g * v1;
    auto const v2 = _ic2eq + t2;

    _ic1eq = v1 + t1;
    _ic2eq = v2 + t2;

    if constexpr (Type == StateVariableFilterType::Highpass) {
        return v0;
    } else if constexpr (Type == StateVariableFilterType::Bandpass) {
        return v1;
    } else if constexpr (Type == StateVariableFilterType::Lowpass) {
        return v2;
    } else if constexpr (Type == StateVariableFilterType::Notch) {
        return v0 + v2;
    } else if constexpr (Type == StateVariableFilterType::Peak) {
        return v0 - v2;
    } else if constexpr (Type == StateVariableFilterType::Allpass) {
        return v0 - _k * v1 + v2;
    } else {
        static_assert(etl::always_false<decltype(Type)>);
    }
}

template<etl::floating_point Float, StateVariableFilterType Type>
auto StateVariableFilter<Float, Type>::reset() -> void
{
    _ic1eq = Float(0);
    _ic2eq = Float(0);
}

template<etl::floating_point Float, StateVariableFilterType Type>
auto StateVariableFilter<Float, Type>::update() -> void
{
    auto w = static_cast<Float>(etl::numbers::pi) * _parameter.cutoff / _sampleRate;
    _g     = etl::tan(w);
    _k     = 1 / _parameter.resonance;

    auto gk = _g + _k;
    _gt0    = 1 / (1 + _g * gk);
    _gk0    = gk * _gt0;
}

}  // namespace grit
