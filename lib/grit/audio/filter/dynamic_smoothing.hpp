#pragma once

#include <etl/algorithm.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>
#include <etl/numbers.hpp>
#include <etl/numeric.hpp>
#include <etl/type_traits.hpp>

namespace grit {

/// \brief Parameter Smoothing
/// \details https://cytomic.com/files/dsp/DynamicSmoothing.pdf
/// \ingroup grit-audio-filter
template<etl::floating_point Float>
struct DynamicSmoothing
{
    DynamicSmoothing() = default;

    auto setSampleRate(Float sampleRate) -> void;
    auto operator()(Float input) -> Float;
    auto reset() -> void;

private:
    Float _baseFrequency{2.0};
    Float _sensitivity{0.5};
    Float _low1{};
    Float _low2{};
    Float _wc{};
    Float _inz{};
};

template<etl::floating_point Float>
auto DynamicSmoothing<Float>::setSampleRate(Float sampleRate) -> void
{
    auto const wc = _baseFrequency / sampleRate;
    _wc           = wc;
}

template<etl::floating_point Float>
auto DynamicSmoothing<Float>::operator()(Float input) -> Float
{
    auto const low1z = _low1;
    auto const low2z = _low2;
    auto const bandz = low1z - low2z;

    auto const x1 = Float(5.9948827);
    auto const x2 = Float(-11.969296);
    auto const x3 = Float(15.959062);

    auto const wd = _wc + _sensitivity * etl::abs(bandz);
    auto const g  = etl::min(wd * (x1 + wd * (x2 + wd * x3)), Float(1));

    _low1 = low1z + g * (Float(0.5) * (input + _inz) - low1z);
    _low2 = low2z + g * (Float(0.5) * (_low1 + low1z) - low2z);
    _inz  = input;

    return _low2;
}

template<etl::floating_point Float>
auto DynamicSmoothing<Float>::reset() -> void
{
    _low1 = Float(0);
    _low2 = Float(0);
    _inz  = Float(0);
}
}  // namespace grit
