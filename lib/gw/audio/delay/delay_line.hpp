#pragma once

#include <gw/math/buffer_interpolation.hpp>

#include <etl/algorithm.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>
#include <etl/mdspan.hpp>

namespace gw {

template<etl::floating_point SampleType, typename Interpolation = BufferInterpolation::Hermite>
struct DelayLine
{
    explicit DelayLine(etl::mdspan<SampleType, etl::dextents<etl::size_t, 1>> buffer) noexcept;

    auto setDelay(SampleType delayInSamples) -> void;

    auto pushSample(SampleType sample) -> void;
    auto popSample() -> SampleType;

    auto reset() -> void;

private:
    SampleType _frac{0};
    etl::size_t _delay{0};
    etl::size_t _writePos{0};
    etl::mdspan<SampleType, etl::dextents<etl::size_t, 1>> _buffer;
    [[no_unique_address]] Interpolation _interpolator{};
};

template<etl::floating_point SampleType, typename Interpolation>
DelayLine<SampleType, Interpolation>::DelayLine(etl::mdspan<SampleType, etl::dextents<etl::size_t, 1>> buffer) noexcept
    : _buffer{buffer}
{}

template<etl::floating_point SampleType, typename Interpolation>
auto DelayLine<SampleType, Interpolation>::setDelay(SampleType delayInSamples) -> void
{
    auto const delay = static_cast<etl::size_t>(delayInSamples);
    _frac            = delayInSamples - static_cast<SampleType>(delay);
    _delay           = etl::clamp<etl::size_t>(delay, 0, _buffer.extent(0) - 1);
}

template<etl::floating_point SampleType, typename Interpolation>
auto DelayLine<SampleType, Interpolation>::pushSample(SampleType sample) -> void
{
    _buffer(_writePos) = sample;
    _writePos          = (_writePos - 1 + _buffer.extent(0)) % _buffer.extent(0);
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
    for (auto i{0}; etl::cmp_less(i, _buffer.extent(0)); ++i) {
        _buffer(i) = SampleType(0);
    }
}

}  // namespace gw
