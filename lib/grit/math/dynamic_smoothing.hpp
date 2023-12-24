#pragma once

#include <etl/algorithm.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>
#include <etl/numbers.hpp>
#include <etl/numeric.hpp>
#include <etl/type_traits.hpp>

namespace grit {
enum struct dynamic_smoothing_type
{
    Efficient,
    Accurate,
};

/// \brief https://cytomic.com/files/dsp/DynamicSmoothing.pdf
template<etl::floating_point Float, dynamic_smoothing_type SmoothingType = dynamic_smoothing_type::Accurate>
struct dynamic_smoothing
{
    dynamic_smoothing() = default;

    auto prepare(Float sample_rate) -> void;
    auto process(Float input) -> Float;
    auto reset() -> void;

private:
    struct efficient
    {
        Float g0{};
        Float sense{};
    };

    struct accurate
    {
        Float wc{};
        Float inz{};
    };

    using State = etl::conditional_t<SmoothingType == dynamic_smoothing_type::Efficient, efficient, accurate>;

    Float _base_frequency{2.0};
    Float _sensitivity{0.5};
    Float _low1{};
    Float _low2{};
    State _state{};
};

template<etl::floating_point Float, dynamic_smoothing_type SmoothingType>
auto dynamic_smoothing<Float, SmoothingType>::prepare(Float sample_rate) -> void
{
    if constexpr (SmoothingType == dynamic_smoothing_type::Efficient) {
        auto const wc = _base_frequency / sample_rate;
        auto const gc = etl::tan(static_cast<Float>(etl::numbers::pi) * wc);

        _state.g0    = Float(2) * gc / (Float(1) + gc);
        _state.sense = _sensitivity * Float(4);
    } else {
        auto const wc = _base_frequency / sample_rate;
        _state.wc     = wc;
    }
}

template<etl::floating_point Float, dynamic_smoothing_type SmoothingType>
auto dynamic_smoothing<Float, SmoothingType>::process(Float input) -> Float
{
    auto const low1z = _low1;
    auto const low2z = _low2;
    auto const bandz = low1z - low2z;

    if constexpr (SmoothingType == dynamic_smoothing_type::Efficient) {
        auto const g = etl::min(_state.g0 + _state.sense + etl::abs(bandz), Float(1));
        _low1        = low1z + g * (input - low1z);
        _low2        = low2z + g * (_low1 - low2z);
    } else {
        auto const x1 = Float(5.9948827);
        auto const x2 = Float(-11.969296);
        auto const x3 = Float(15.959062);

        auto const wd = _state.wc + _sensitivity * etl::abs(bandz);
        auto const g  = etl::min(wd * (x1 + wd * (x2 + wd * x3)), Float(1));

        _low1      = low1z + g * (Float(0.5) * (input + _state.inz) - low1z);
        _low2      = low2z + g * (Float(0.5) * (_low1 + low1z) - low2z);
        _state.inz = input;
    }

    return _low2;
}

template<etl::floating_point Float, dynamic_smoothing_type SmoothingType>
auto dynamic_smoothing<Float, SmoothingType>::reset() -> void
{
    _low1 = Float(0);
    _low2 = Float(0);
    if constexpr (SmoothingType == dynamic_smoothing_type::Accurate) {
        _state.inz = Float(0);
    }
}
}  // namespace grit
