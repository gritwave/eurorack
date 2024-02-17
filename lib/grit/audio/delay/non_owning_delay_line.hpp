#pragma once

#include <grit/math/buffer_interpolation.hpp>

#include <etl/cmath.hpp>
#include <etl/concepts.hpp>
#include <etl/mdspan.hpp>

namespace grit {

/// \ingroup grit-audio-delay
template<etl::floating_point Float, typename Extent, typename Interpolation = BufferInterpolation::Hermite>
struct NonOwningDelayLine
{
    explicit NonOwningDelayLine(etl::mdspan<Float, Extent> buffer);

    auto setDelay(Float delayInSamples) -> void;

    auto pushSample(Float sample) -> void;
    auto popSample() -> Float;

    auto reset() -> void;

private:
    Float _frac{0};
    etl::size_t _delay{0};
    etl::size_t _writePos{0};
    etl::mdspan<Float, Extent> _buffer;
    [[no_unique_address]] Interpolation _interpolator{};
};

template<etl::floating_point Float, typename Extent, typename Interpolation>
NonOwningDelayLine<Float, Extent, Interpolation>::NonOwningDelayLine(etl::mdspan<Float, Extent> buffer)
    : _buffer{buffer}
{}

template<etl::floating_point Float, typename Extent, typename Interpolation>
auto NonOwningDelayLine<Float, Extent, Interpolation>::setDelay(Float delayInSamples) -> void
{
    auto const delay = static_cast<etl::size_t>(delayInSamples);
    _frac            = delayInSamples - static_cast<Float>(delay);
    _delay           = etl::clamp<etl::size_t>(delay, 0, _buffer.extent(0) - 1);
}

template<etl::floating_point Float, typename Extent, typename Interpolation>
auto NonOwningDelayLine<Float, Extent, Interpolation>::pushSample(Float sample) -> void
{
    _buffer(_writePos) = sample;
    _writePos          = (_writePos - 1 + _buffer.extent(0)) % _buffer.extent(0);
}

template<etl::floating_point Float, typename Extent, typename Interpolation>
auto NonOwningDelayLine<Float, Extent, Interpolation>::popSample() -> Float
{
    auto const readPos = _writePos + _delay;
    return _interpolator(_buffer, readPos, _frac);
}

template<etl::floating_point Float, typename Extent, typename Interpolation>
auto NonOwningDelayLine<Float, Extent, Interpolation>::reset() -> void
{
    _writePos = 0;
    for (auto i{0}; etl::cmp_less(i, _buffer.extent(0)); ++i) {
        _buffer(i) = Float(0);
    }
}

}  // namespace grit
