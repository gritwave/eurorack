#pragma once

#include <gw/audio/delay/delay_line.hpp>
#include <gw/math/buffer_interpolation.hpp>

#include <etl/array.hpp>
#include <etl/concepts.hpp>

namespace gw
{

template<etl::floating_point SampleType, etl::size_t MaxDelay, typename Interpolation = BufferInterpolation::Hermite>
struct StaticDelayLine
{
    StaticDelayLine() = default;

    auto setDelay(SampleType delayInSamples) -> void;

    auto pushSample(SampleType sample) -> void;
    auto popSample() -> SampleType;

    auto reset() -> void;

private:
    etl::array<SampleType, MaxDelay> _buffer;
    DelayLine<SampleType, Interpolation> _delayLine{_buffer};
};

template<etl::floating_point SampleType, etl::size_t MaxDelay, typename Interpolation>
auto StaticDelayLine<SampleType, MaxDelay, Interpolation>::setDelay(SampleType delayInSamples) -> void
{
    _delayLine.setDelay(delayInSamples);
}

template<etl::floating_point SampleType, etl::size_t MaxDelay, typename Interpolation>
auto StaticDelayLine<SampleType, MaxDelay, Interpolation>::pushSample(SampleType sample) -> void
{
    _delayLine.pushSample(sample);
}

template<etl::floating_point SampleType, etl::size_t MaxDelay, typename Interpolation>
auto StaticDelayLine<SampleType, MaxDelay, Interpolation>::popSample() -> SampleType
{
    return _delayLine.popSample();
}

template<etl::floating_point SampleType, etl::size_t MaxDelay, typename Interpolation>
auto StaticDelayLine<SampleType, MaxDelay, Interpolation>::reset() -> void
{
    _delayLine.reset();
}

}  // namespace gw
