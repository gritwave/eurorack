#pragma once

#include <mc/audio/delay/delay_interpolation.hpp>
#include <mc/math/fast_lerp.hpp>
#include <mc/math/hermite_interpolation.hpp>

#include <etl/algorithm.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>
#include <etl/span.hpp>

namespace mc::audio
{

template<etl::floating_point SampleType, DelayInterpolation Interpolation = DelayInterpolation::Hermite>
struct NonOwningDelayLine
{
    explicit NonOwningDelayLine(etl::span<SampleType> buffer) noexcept;

    auto setDelay(SampleType delayInSamples) -> void;

    auto pushSample(SampleType sample) -> void;
    auto popSample() -> SampleType;

    auto reset() -> void;

private:
    SampleType _frac{0};
    etl::size_t _delay{0};
    etl::size_t _writePos{0};
    etl::span<SampleType> _buffer;
};

template<etl::floating_point SampleType, DelayInterpolation Interpolation>
NonOwningDelayLine<SampleType, Interpolation>::NonOwningDelayLine(etl::span<SampleType> buffer) noexcept
    : _buffer{buffer}
{
}

template<etl::floating_point SampleType, DelayInterpolation Interpolation>
auto NonOwningDelayLine<SampleType, Interpolation>::setDelay(SampleType delayInSamples) -> void
{
    auto const delay = static_cast<etl::size_t>(delayInSamples);
    _frac            = delayInSamples - static_cast<SampleType>(delay);
    _delay           = etl::clamp<etl::size_t>(delay, 0, _buffer.size() - 1);
}

template<etl::floating_point SampleType, DelayInterpolation Interpolation>
auto NonOwningDelayLine<SampleType, Interpolation>::pushSample(SampleType sample) -> void
{
    _buffer[_writePos] = sample;
    _writePos          = (_writePos - 1 + _buffer.size()) % _buffer.size();
}

template<etl::floating_point SampleType, DelayInterpolation Interpolation>
auto NonOwningDelayLine<SampleType, Interpolation>::popSample() -> SampleType
{
    if constexpr (Interpolation == DelayInterpolation::None)
    {
        auto const readPos = _writePos + _delay;
        return _buffer[readPos % _buffer.size()];
    }
    else if constexpr (Interpolation == DelayInterpolation::Linear)
    {
        auto const readPos = _writePos + _delay;
        auto const x0      = _buffer[readPos % _buffer.size()];
        auto const x1      = _buffer[(readPos + 1) % _buffer.size()];
        return fast_lerp(x0, x1, _frac);
    }
    else
    {
        auto const readPos = static_cast<etl::ptrdiff_t>(_writePos + _delay);
        auto const xm1     = _buffer[(readPos - 1) % _buffer.size()];
        auto const x0      = _buffer[readPos % _buffer.size()];
        auto const x1      = _buffer[(readPos + 1) % _buffer.size()];
        auto const x2      = _buffer[(readPos + 2) % _buffer.size()];
        return hermite_interpolation(xm1, x0, x1, x2, _frac);
    }
}

template<etl::floating_point SampleType, DelayInterpolation Interpolation>
auto NonOwningDelayLine<SampleType, Interpolation>::reset() -> void
{
    _writePos = 0;
    etl::fill(_buffer.begin(), _buffer.end(), SampleType{0});
}

}  // namespace mc::audio
