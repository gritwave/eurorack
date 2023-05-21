#pragma once

#include <etl/algorithm.hpp>
#include <etl/numbers.hpp>
#include <etl/numeric.hpp>

#include <cmath>

namespace digitaldreams
{

/// \brief https://cytomic.com/files/dsp/DynamicSmoothing.pdf
template<typename SampleType>
struct DynamicSmoothing
{
    DynamicSmoothing() = default;

    auto prepare(SampleType sampleRate) -> void;
    auto process(SampleType input) -> SampleType;
    auto reset() -> void;

private:
    SampleType _baseFrequency{2.0};
    SampleType _sensitivity{0.5};

    SampleType _low1{};
    SampleType _low2{};
    SampleType _g0{};
    SampleType _sense{};
};

template<typename SampleType>
auto DynamicSmoothing<SampleType>::prepare(SampleType sampleRate) -> void
{
    auto const wc = _baseFrequency / sampleRate;
    auto const gc = std::tan(static_cast<SampleType>(etl::numbers::pi) * wc);

    _g0    = SampleType(2) * gc / (SampleType(1) + gc);
    _sense = _sensitivity * SampleType(4);
}

template<typename SampleType>
auto DynamicSmoothing<SampleType>::process(SampleType input) -> SampleType
{
    auto const low1z = _low1;
    auto const low2z = _low2;
    auto const bandz = low1z - low2z;
    auto const g     = etl::min(_g0 + _sense + etl::abs(bandz), SampleType(1));
    _low1            = low1z + g * (input - low1z);
    _low2            = low2z + g * (_low1 - low2z);
    return _low2;
}

template<typename SampleType>
auto DynamicSmoothing<SampleType>::reset() -> void
{
    _low1 = SampleType(0);
    _low2 = SampleType(0);
}

}  // namespace digitaldreams
