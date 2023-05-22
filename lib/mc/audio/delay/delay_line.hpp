#pragma once

#include <mc/audio/delay/delay_interpolation.hpp>

#include <etl/algorithm.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>
#include <etl/span.hpp>

namespace mc::audio
{

template<etl::floating_point SampleType, typename Interpolation = DelayInterpolation::Hermite>
struct DelayLine
{
    explicit DelayLine(etl::span<SampleType> buffer) noexcept;

    auto setDelay(SampleType delayInSamples) -> void;

    auto pushSample(SampleType sample) -> void;
    auto popSample() -> SampleType;

    auto reset() -> void;

private:
    SampleType _frac{0};
    etl::size_t _delay{0};
    etl::size_t _writePos{0};
    etl::span<SampleType> _buffer;
    [[no_unique_address]] Interpolation _interpolator{};
};

template<etl::floating_point SampleType, typename Interpolation>
DelayLine<SampleType, Interpolation>::DelayLine(etl::span<SampleType> buffer) noexcept : _buffer{buffer}
{
}

template<etl::floating_point SampleType, typename Interpolation>
auto DelayLine<SampleType, Interpolation>::setDelay(SampleType delayInSamples) -> void
{
    auto const delay = static_cast<etl::size_t>(delayInSamples);
    _frac            = delayInSamples - static_cast<SampleType>(delay);
    _delay           = etl::clamp<etl::size_t>(delay, 0, _buffer.size() - 1);
}

template<etl::floating_point SampleType, typename Interpolation>
auto DelayLine<SampleType, Interpolation>::pushSample(SampleType sample) -> void
{
    _buffer[_writePos] = sample;
    _writePos          = (_writePos - 1 + _buffer.size()) % _buffer.size();
}

template<etl::floating_point SampleType, typename Interpolation>
auto DelayLine<SampleType, Interpolation>::popSample() -> SampleType
{
    auto const readPos = _writePos + _delay;
    return _interpolator(_buffer, readPos, _frac);
}

template<etl::floating_point SampleType, typename Interpolation>
auto DelayLine<SampleType, Interpolation>::reset() -> void
{
    _writePos = 0;
    etl::fill(_buffer.begin(), _buffer.end(), SampleType{0});
}

}  // namespace mc::audio
