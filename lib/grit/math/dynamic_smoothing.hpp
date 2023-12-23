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
template<etl::floating_point SampleType, dynamic_smoothing_type SmoothingType = dynamic_smoothing_type::Accurate>
struct dynamic_smoothing
{
    dynamic_smoothing() = default;

    auto prepare(SampleType sample_rate) -> void;
    auto process(SampleType input) -> SampleType;
    auto reset() -> void;

private:
    struct efficient
    {
        SampleType g0{};
        SampleType sense{};
    };

    struct accurate
    {
        SampleType wc{};
        SampleType inz{};
    };

    using State = etl::conditional_t<SmoothingType == dynamic_smoothing_type::Efficient, efficient, accurate>;

    SampleType _base_frequency{2.0};
    SampleType _sensitivity{0.5};
    SampleType _low1{};
    SampleType _low2{};
    State _state{};
};

template<etl::floating_point SampleType, dynamic_smoothing_type SmoothingType>
auto dynamic_smoothing<SampleType, SmoothingType>::prepare(SampleType sample_rate) -> void
{
    if constexpr (SmoothingType == dynamic_smoothing_type::Efficient) {
        auto const wc = _base_frequency / sample_rate;
        auto const gc = etl::tan(static_cast<SampleType>(etl::numbers::pi) * wc);

        _state.g0    = SampleType(2) * gc / (SampleType(1) + gc);
        _state.sense = _sensitivity * SampleType(4);
    } else {
        auto const wc = _base_frequency / sample_rate;
        _state.wc     = wc;
    }
}

template<etl::floating_point SampleType, dynamic_smoothing_type SmoothingType>
auto dynamic_smoothing<SampleType, SmoothingType>::process(SampleType input) -> SampleType
{
    auto const low1z = _low1;
    auto const low2z = _low2;
    auto const bandz = low1z - low2z;

    if constexpr (SmoothingType == dynamic_smoothing_type::Efficient) {
        auto const g = etl::min(_state.g0 + _state.sense + etl::abs(bandz), SampleType(1));
        _low1        = low1z + g * (input - low1z);
        _low2        = low2z + g * (_low1 - low2z);
    } else {
        auto const x1 = SampleType(5.9948827);
        auto const x2 = SampleType(-11.969296);
        auto const x3 = SampleType(15.959062);

        auto const wd = _state.wc + _sensitivity * etl::abs(bandz);
        auto const g  = etl::min(wd * (x1 + wd * (x2 + wd * x3)), SampleType(1));

        _low1      = low1z + g * (SampleType(0.5) * (input + _state.inz) - low1z);
        _low2      = low2z + g * (SampleType(0.5) * (_low1 + low1z) - low2z);
        _state.inz = input;
    }

    return _low2;
}

template<etl::floating_point SampleType, dynamic_smoothing_type SmoothingType>
auto dynamic_smoothing<SampleType, SmoothingType>::reset() -> void
{
    _low1 = SampleType(0);
    _low2 = SampleType(0);
    if constexpr (SmoothingType == dynamic_smoothing_type::Accurate) {
        _state.inz = SampleType(0);
    }
}
}  // namespace grit
